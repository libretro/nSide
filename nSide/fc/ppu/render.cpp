auto PPU::enable() const -> bool {
  return io.bgEnable || io.objEnable;
}

auto PPU::loadCHR(uint14 addr) -> uint8 {
  if(!enable()) return 0x00;
  return bus.readCHR(io.chrAddressBus = addr, io.mdr);
}

auto PPU::renderPixel() -> void {
  if(vcounter() >= 240) return;
  uint x = hcounter() - 1;
  uint mask = 0x8000 >> (io.v.fineX + (x & 7));
  uint palette = 0;
  uint objectPalette = 0;
  bool objectPriority = 0;

  palette |= l.tiledataLo & mask ? 1 : 0;
  palette |= l.tiledataHi & mask ? 2 : 0;
  if(palette) {
    uint attr = l.attribute;
    if(mask >= 256) attr >>= 2;
    palette |= (attr & 3) << 2;
  }

  if(!io.bgEnable) palette = 0;
  if(!io.bgEdgeEnable && x < 8) palette = 0;

  if(io.objEnable)
  for(int sprite = 7; sprite >= 0; sprite--) {
    if(!io.objEdgeEnable && x < 8) continue;
    if(l.oam[sprite].id == 64) continue;

    uint spriteX = x - l.oam[sprite].x;
    if(spriteX >= 8) continue;

    if(l.oam[sprite].attr & 0x40) spriteX ^= 7;
    uint mask = 0x80 >> spriteX;
    uint spritePalette = 0;
    spritePalette |= l.oam[sprite].tiledataLo & mask ? 1 : 0;
    spritePalette |= l.oam[sprite].tiledataHi & mask ? 2 : 0;
    if(spritePalette == 0) continue;

    if(l.oam[sprite].id == 0 && palette && x != 255) io.spriteZeroHit = 1;
    spritePalette |= (l.oam[sprite].attr & 3) << 2;

    objectPriority = l.oam[sprite].attr & 0x20;
    objectPalette = 16 + spritePalette;
  }

  if(objectPalette) {
    if(palette == 0 || objectPriority == 0) palette = objectPalette;
  }

//const uint offset = side << 9;
//if(vcounter() == 0 && x == 0) raster.reset();
//if(x == 0) raster.scanline();

  if(pal() && (x < 2 || x >= 254 || vcounter() <= 0)) {
  //return raster.pixel(offset + ((io.emphasis << 6) | 0x1d));
    output[vcounter() * 256 + x] = (io.emphasis << 6) | 0x1d;
    return;
  }

  if(!enable()) {
    if((io.v.addressHi & 0x3f) != 0x3f) {
      palette = extIn();
    } else {
      palette = io.v.addressLo;
    }
    _extOut = io.v.addressLo;
  } else {
    if(!palette) palette = extIn();
    _extOut = 0;
  }
  if(!Model::PlayChoice10() || playchoice10.ppuOutput) {
  //raster.pixel(offset + (io.emphasis << 6 | readCGRAM(palette)));
    output[vcounter() * 256 + x] = io.emphasis << 6 | readCGRAM(palette);
  } else {
  //raster.pixel(offset + 0x0f);
    output[vcounter() * 256 + x] = 0x0f;
  }
}

auto PPU::renderSprite() -> void {
  if(!enable()) return;

  uint n = l.oamIterator++;
  int ly = vcounter() == vlines() - 1 ? -1 : (int)vcounter();
  uint y = ly - oam[n * 4 + 0];

  if(y >= io.spriteHeight) return;
  if(l.oamCounter == 8) {
    io.spriteOverflow = 1;
    return;
  }

  auto& o = l.soam[l.oamCounter++];
  o.id   = n;
  o.y    = oam[n * 4 + 0];
  o.tile = oam[n * 4 + 1];
  o.attr = oam[n * 4 + 2];
  o.x    = oam[n * 4 + 3];
}

auto PPU::renderScanline() -> void {
  //Vblank
  if((vcounter() >= 240 && vcounter() < vlines() - 1)) return step(341), scanline();

  l.oamIterator = 0;
  l.oamCounter = 0;

  for(auto n : range(8)) l.soam[n] = {};

  //  0
  step(1);

  //  1-256
  for(uint tile : range(32)) {
    uint nametable = loadCHR(0x2000 | (uint12)io.v.address);
    uint tileaddr = io.bgAddress | nametable << 4 | io.v.fineY;
    renderPixel();
    step(1);

    renderPixel();
    step(1);

    uint attribute = loadCHR(0x23c0 | io.v.nametable << 10 | (io.v.tileY >> 2) << 3 | io.v.tileX >> 2);
    if(io.v.tileY & 2) attribute >>= 4;
    if(io.v.tileX & 2) attribute >>= 2;
    renderPixel();
    step(1);

    if(enable() && ++io.v.tileX == 0) io.v.nametableX ^= 1;
    if(enable() && tile == 31 && ++io.v.fineY == 0 && ++io.v.tileY == 30) io.v.nametableY ^= 1, io.v.tileY = 0;
    renderPixel();
    renderSprite();
    step(1);

    uint tiledataLo = loadCHR(tileaddr + 0);
    renderPixel();
    step(1);

    renderPixel();
    step(1);

    uint tiledataHi = loadCHR(tileaddr + 8);
    renderPixel();
    step(1);

    renderPixel();
    renderSprite();
    step(1);

    l.nametable = l.nametable << 8 | nametable;
    l.attribute = l.attribute << 2 | (attribute & 3);
    l.tiledataLo = l.tiledataLo << 8 | tiledataLo;
    l.tiledataHi = l.tiledataHi << 8 | tiledataHi;
  }

  for(auto n : range(8)) l.oam[n] = l.soam[n];

  //257-320
  for(uint sprite : range(8)) {
    uint nametable = loadCHR(0x2000 | (uint12)io.v.address);
    step(1);

    if(enable() && sprite == 0) {
      //258
      io.v.nametableX = io.t.nametableX;
      io.v.tileX = io.t.tileX;
    }
    step(1);

    uint attribute = loadCHR(0x23c0 | io.v.nametable << 10 | (io.v.tileY >> 2) << 3 | io.v.tileX >> 2);
    uint tileaddr = io.spriteHeight == 8
    ? io.objAddress + l.oam[sprite].tile * 16
    : (l.oam[sprite].tile & ~1) * 16 + (l.oam[sprite].tile & 1) * 0x1000;
    step(2);

    uint spriteY = (vcounter() - l.oam[sprite].y) & (io.spriteHeight - 1);
    if(l.oam[sprite].attr & 0x80) spriteY ^= io.spriteHeight - 1;
    tileaddr += spriteY + (spriteY & 8);

    l.oam[sprite].tiledataLo = loadCHR(tileaddr + 0);
    step(2);

    l.oam[sprite].tiledataHi = loadCHR(tileaddr + 8);
    step(2);

    if(enable() && sprite == 6 && vcounter() == vlines() - 1) {
      //305
      io.v.address = io.t.address;
    }
  }

  //321-336
  for(uint tile : range(2)) {
    uint nametable = loadCHR(0x2000 | (uint12)io.v.address);
    uint tileaddr = io.bgAddress | nametable << 4 | io.v.fineY;
    step(2);

    uint attribute = loadCHR(0x23c0 | io.v.nametable << 10 | (io.v.tileY >> 2) << 3 | io.v.tileX >> 2);
    if(io.v.tileY & 2) attribute >>= 4;
    if(io.v.tileX & 2) attribute >>= 2;
    step(1);

    if(enable() && ++io.v.tileX == 0) io.v.nametableX ^= 1;
    step(1);

    uint tiledataLo = loadCHR(tileaddr + 0);
    step(2);

    uint tiledataHi = loadCHR(tileaddr + 8);
    step(2);

    l.nametable = l.nametable << 8 | nametable;
    l.attribute = l.attribute << 2 | (attribute & 3);
    l.tiledataLo = l.tiledataLo << 8 | tiledataLo;
    l.tiledataHi = l.tiledataHi << 8 | tiledataHi;
  }

  //337-338
  loadCHR(0x2000 | (uint12)io.v.address);
  step(2);

  //339
  loadCHR(0x2000 | (uint12)io.v.address);
  step(1);

  //340
  if(hcounter() > 0) step(1);

  return scanline();
}
