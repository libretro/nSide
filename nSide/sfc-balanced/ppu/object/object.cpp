#include "oam.cpp"

auto PPU::Object::addressReset() -> void {
  ppu.io.oamAddress = ppu.io.oamBaseAddress;
  setFirstSprite();
}

auto PPU::Object::setFirstSprite() -> void {
  io.firstSprite = !ppu.io.oamPriority ? 0 : (ppu.io.oamAddress >> 2) & 127;
}

auto PPU::Object::power() -> void {
  for(auto& object : oam.object) {
    object.x = 0;
    object.y = 0;
    object.character = 0;
    object.nameselect = 0;
    object.vflip = 0;
    object.hflip = 0;
    object.priority = 0;
    object.palette = 0;
    object.size = 0;
  }

  t.itemCount = 0;
  t.tileCount = 0;

  io.aboveEnable = random();
  io.belowEnable = random();
  io.interlace = random();

  io.baseSize = random();
  io.nameselect = random();
  io.tiledataAddress = (random() & 7) << 13;
  io.firstSprite = 0;

  for(auto& p : io.priority) p = 0;

  io.timeOver = false;
  io.rangeOver = false;
}

auto PPU::Object::onScanline(uint id) -> bool {
  //if sprite is entirely offscreen and doesn't wrap around to the left side of the screen,
  //then it is not counted. this *should* be 256, and not 255, even though dot 256 is offscreen.
  OAM::Object& spr = oam.object[id];
  if(spr.x > 256 && (spr.x + spr.width - 1) < 512) return false;

  int height = !io.interlace ? (uint)spr.height : spr.height >> 1;
  if(ppu.vcounter() >= spr.y && ppu.vcounter() < (spr.y + height)) return true;
  if((spr.y + height) >= 256 && ppu.vcounter() < ((spr.y + height) & 255)) return true;
  return false;
}

auto PPU::Object::loadTiles(uint id) -> void {
  OAM::Object& spr = oam.object[id];
  uint16 tile_width = spr.width >> 3;
  int x = spr.x;
  int y = (ppu.vcounter() - spr.y) & 0xff;
  if(io.interlace) {
    y <<= 1;
  }

  if(spr.vflip) {
    if(spr.width == spr.height) {
      y = (spr.height - 1) - y;
    } else {
      y = y < spr.width ? (spr.width - 1) - y : spr.width + ((spr.width - 1) - (y - spr.width));
    }
  }

  if(io.interlace) {
    y = !spr.vflip ? y + ppu.field() : y - ppu.field();
  }

  x &= 511;
  y &= 255;

  uint16 tdaddr = cache.tiledataAddress;
  uint16 chrx   = (spr.character     ) & 15;
  uint16 chry   = (spr.character >> 4) & 15;
  if(spr.nameselect) {
    tdaddr += (256 * 16) + (cache.nameselect << 12);
  }
  chry  += (y >> 3);
  chry  &= 15;
  chry <<= 4;

  for(uint tx : range(tile_width)) {
    uint sx = (x + (tx << 3)) & 511;
    //ignore sprites that are offscreen, x==256 is a special case that loads all tiles in OBJ
    if(x != 256 && sx >= 256 && (sx + 7) < 512) continue;

    if(t.tileCount++ >= 34) break;
    uint n = t.tileCount - 1;
    tileList[n].x        = sx;
    tileList[n].y        = y;
    tileList[n].priority = spr.priority;
    tileList[n].palette  = 128 + (spr.palette << 4);
    tileList[n].hflip    = spr.hflip;

    uint mx  = !spr.hflip ? tx : (tile_width - 1) - tx;
    uint pos = tdaddr + ((chry + ((chrx + mx) & 15)) << 4);
    tileList[n].tile = (pos >> 4) & (ppu.vram.mask >> 4);
  }
}

auto PPU::Object::renderTile(uint n) -> void {
  uint8* tile = ppu.cache.tile(Background::Mode::BPP4, tileList[n].tile);

  uint sx = tileList[n].x;
  uint8* tile_ptr = (uint8*)tile + ((tileList[n].y & 7) << 3);
  for(uint x : range(8)) {
    sx &= 511;
    if(sx < 256) {
      uint col = *(tile_ptr + (!tileList[n].hflip ? x : 7 - x));
      if(col) {
        col += tileList[n].palette;
        output.palette[sx] = col;
        output.priority[sx] = tileList[n].priority;
      }
    }
    sx++;
  }
}

auto PPU::Object::render() -> void {
  if(io.priority[0] + io.priority[1] + io.priority[2] + io.priority[3] == 0) return;

  if(!io.aboveEnable && !io.belowEnable) return;

  for(uint s : range(34)) {
    if(tileList[s].tile == 0xffff) continue;
    renderTile(s);
  }

  ppu.window.buildTables(ID::OBJ);
  uint8* wt_above = ppu.window.cache[ID::OBJ].above;
  uint8* wt_below = ppu.window.cache[ID::OBJ].below;

  for(int x : range(256)) {
    if(output.priority[x] == PRIORITY_NONE) continue;

    uint16 color = ppu.screen.cgram[output.palette[x]];
    uint pri = io.priority[output.priority[x]];
    uint source = ID::OBJ;
    bool exemption = output.palette[x] < 192;
    if(io.aboveEnable && !wt_above[x]) ppu.screen.output.plotAbove(x, color, pri, source, exemption);
    if(io.belowEnable && !wt_below[x]) ppu.screen.output.plotBelow(x, color, pri, source, exemption);
  }
}

auto PPU::Object::renderRTO() -> void {
  t.itemCount = 0;
  t.tileCount = 0;

  memset(output.priority, PRIORITY_NONE, 256);
  memset(itemList, 0xff, 32);
  for(int s : range(34)) tileList[s].tile = 0xffff;

  for(int i : range(128)) {
    uint s = (i + io.firstSprite) & 127;
    if(!onScanline(s)) continue;
    if(t.itemCount++ >= 32) break;
    itemList[t.itemCount - 1] = s;
  }

  if(t.itemCount > 0 && itemList[t.itemCount - 1] != 0xff) {
    ppu.latch.oamAddress = 0x0200 + (itemList[t.itemCount - 1] >> 2);
  }

  for(int s = 31; s >= 0; s--) {
    if(itemList[s] == 0xff) continue;
    loadTiles(itemList[s]);
  }

  io.timeOver  |= (t.tileCount > 34);
  io.rangeOver |= (t.itemCount > 32);
}
