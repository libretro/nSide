#include "mode7.cpp"
uint4 PPU::Background::Mosaic::size;

auto PPU::Background::hires() const -> bool {
  return ppu.io.bgMode == 5 || ppu.io.bgMode == 6;
}

auto PPU::Background::voffset() const -> uint16 {
  return mosaic.enable ? latch.voffset : io.voffset;
}

auto PPU::Background::power() -> void {
  io = {};
  io.tiledataAddress = (random() & 0x0f) << 12;
  io.screenAddress = (random() & 0xfc) << 8;
  io.screenSize = random();
  io.tileSize = random();
  io.aboveEnable = random();
  io.belowEnable = random();
  io.hoffset = random();
  io.voffset = random();

  latch = {};

  mosaic = {};
  mosaic.size = random();
  mosaic.enable = random();

  //x = 0;
  y = 0;

  tile = 0;
  priority = 0;
  paletteNumber = 0;
  paletteIndex = 0;
}

auto PPU::Background::getTile(uint x, uint y) -> uint16 {
  x = (x & maskX) >> tileWidth;
  y = (y & maskY) >> tileHeight;

  uint16 offset = (y & 0x1f) << 5 | (x & 0x1f);
  if(x & 0x20) offset += screenX;
  if(y & 0x20) offset += screenY;

  const uint16 address = io.screenAddress + offset;
  return ppu.vram[address];
}

//called once at the start of every rendered scanline
auto PPU::Background::scanline() -> void {
  tileHeight = io.tileSize == TileSize::Size8x8 ? 3 : 4;
  tileWidth = !hires() ? uint(tileHeight) : 4;
  uint width = !hires() ? 256 : 512;
  maskX = tileHeight == 3 ? width : width << 1;
  maskY = maskX;
  if(io.screenSize & 1) maskX <<= 1;
  if(io.screenSize & 2) maskY <<= 1;
  maskX--;
  maskY--;

  screenX = io.screenSize & 1 ? 32 << 5 : 0;
  screenY = io.screenSize & 2 ? 32 << 5 : 0;
  if(io.screenSize == ScreenSize::Size64x64) screenY <<= 1;
}

auto PPU::Background::begin() -> void {
  y = ppu.vcounter();

  if(y == 1) {
    mosaic.vcounter = mosaic.size + 1;
    mosaic.voffset = 1;
    latch.hoffset = io.hoffset;
    latch.voffset = io.voffset;
  } else if(--mosaic.vcounter == 0) {
    mosaic.vcounter = mosaic.size + 1;
    mosaic.voffset += mosaic.size + 1;
    latch.hoffset = io.hoffset;
    latch.voffset = io.voffset;
  }

//mosaic.hcounter = mosaic.size + 1;
//mosaic.hoffset = 0;

  if(io.mode == Mode::Mode7) return beginMode7();
  if(!mosaic.enable) {
    latch.hoffset = io.hoffset;
    latch.voffset = io.voffset;
  }
}

auto PPU::Background::directColor(uint8 palette, uint16 tile) const -> uint16 {
  //palette = -------- BBGGGRRR
  //tile    = ---bgr-- --------
  //output  = 0BBb00GG Gg0RRRr0
  return ((palette << 7) & 0x6000) + ((tile >> 0) & 0x1000)
       + ((palette << 4) & 0x0380) + ((tile >> 5) & 0x0040)
       + ((palette << 2) & 0x001c) + ((tile >> 9) & 0x0002);
}

auto PPU::Background::render() -> void {
  if(io.mode == Mode::Inactive) return;
  if(io.priority[0] + io.priority[1] == 0) return;
  if(!io.aboveEnable && !io.belowEnable) return;

  if(io.mode == Mode::Mode7) return renderMode7();

  const uint16 validMask = id == ID::BG1 ? 0x2000 : 0x4000;

  const uint paletteOffset = ppu.io.bgMode == 0 ? id << 5 : 0;
  const uint paletteSize = 2 << io.mode;
  const uint tileMask = ppu.vram.mask >> 3 + io.mode;
  const uint tiledataIndex = io.tiledataAddress >> 3 + io.mode;

//const uint8* bg_tiledata  = ppu.cache.tiledata[io.mode];
//const uint8* bg_tilevalid = ppu.cache.tilevalid[io.mode];

  uint py = mosaic.enable ? (uint)mosaic.voffset : y;

  uint16 hscroll = io.hoffset;
  uint16 vscroll = voffset();

  const uint width = 256 << hires();

  if(hires()) {
    hscroll <<= 1;
    if(ppu.io.interlace) py = py << 1 | (ppu.field() & !mosaic.enable);  //todo: temporary vmosaic hack
  }

  const uint8*  tile_ptr;
  const uint16* mtable = ppu.mosaicTableLo[0];
  if(mosaic.enable) mtable = (hires() ? ppu.mosaicTableHi : ppu.mosaicTableLo)[mosaic.size];

  const bool isOPTMode = ppu.io.bgMode == 2 || ppu.io.bgMode == 4 || ppu.io.bgMode == 6;
  const bool isDirectColorMode = ppu.screen.io.directColor && id == ID::BG1 && (ppu.io.bgMode == 3 || ppu.io.bgMode == 4);

  ppu.window.buildTables(id);
  const uint8* wt_above = ppu.window.cache[id].above;
  const uint8* wt_below = ppu.window.cache[id].below;

  uint16 hval, vval;
  uint16 hoffset, voffset, offsetX, col;
  bool   mirrorX, mirrorY;

  uint16 prev_x = 0xffff, prev_y = 0xffff, prev_optx = 0xffff;
  for(uint16 px : range(width)) {
    hoffset = hscroll + mtable[px];
    voffset = vscroll + py;

    if(isOPTMode) {
      offsetX = px + (hscroll & 7);

      //tile 0 is unaffected by offset-per-tile mode...
      if(offsetX >= 8) {

        //cache tile data in hval, vval if possible
        if((offsetX >> 3) != (prev_optx >> 3)) {
          prev_optx = offsetX;

          hval = ppu.bg3.getTile((offsetX - 8) + (ppu.bg3.io.hoffset & ~7), ppu.bg3.io.voffset);
          if(ppu.io.bgMode != 4) {
            vval = ppu.bg3.getTile((offsetX - 8) + (ppu.bg3.io.hoffset & ~7), ppu.bg3.io.voffset + 8);
          }
        }

        if(ppu.io.bgMode == 4) {
          if(hval & validMask) {
            if(!(hval & 0x8000)) {
              hoffset = offsetX + (hval & ~7);
            } else {
              voffset = py + hval;
            }
          }
        } else {
          if(hval & validMask) hoffset = offsetX + (hval & ~7);
          if(vval & validMask) voffset = py + vval;
        }
      }
    }

    hoffset &= maskX;
    voffset &= maskY;

    if(hoffset >> 3 != prev_x || voffset >> 3 != prev_y) {
      prev_x = hoffset >> 3;
      prev_y = voffset >> 3;

      tile = getTile((uint)hoffset, (uint)voffset);  //format = vhopppcc cccccccc
      mirrorY = tile.bit(15);
      mirrorX = tile.bit(14);
      priority = io.priority[tile.bit(13)];
      paletteNumber = tile.bits(10,12);
      paletteIndex = paletteOffset + (paletteNumber << paletteSize);

      if(tileWidth  == 4 && (bool)(hoffset & 8) != mirrorX) tile +=  1;
      if(tileHeight == 4 && (bool)(voffset & 8) != mirrorY) tile += 16;
      uint16 character = tile.bits(0,9) + tiledataIndex & tileMask;

      //if(bg_tilevalid[character] == 0) {
      //  ppu.cache.tile(io.mode, character);
      //}

      if(mirrorY) voffset ^= 7;  //invert y tile pos
      tile_ptr = ppu.cache.tile(io.mode, character) + ((voffset & 7) << 3);
    }

    if(mirrorX) hoffset ^= 7;  //invert x tile pos
    col = *(tile_ptr + (hoffset & 7));
    if(col) {
      if(isDirectColorMode) {
        col = directColor(col, tile);
      } else {
        col = ppu.screen.cgram[col | paletteIndex];
      }

      if(!hires()) {
        if(io.aboveEnable && !wt_above[px]) ppu.screen.output.plotAbove(px, col, priority, id, false);
        if(io.belowEnable && !wt_below[px]) ppu.screen.output.plotBelow(px, col, priority, id, false);
      } else {
        int hx = px >> 1;
        if(px & 1) {
          if(io.aboveEnable && !wt_above[hx]) ppu.screen.output.plotAbove(hx, col, priority, id, false);
        } else {
          if(io.belowEnable && !wt_below[hx]) ppu.screen.output.plotBelow(hx, col, priority, id, false);
        }
      }
    }
  }
}
