PPU::Cache::Cache() {
  tiledata[Background::Mode::BPP2]  = new uint8[8192 * 64]();
  tiledata[Background::Mode::BPP4]  = new uint8[4096 * 64]();
  tiledata[Background::Mode::BPP8]  = new uint8[2048 * 64]();
  tilevalid[Background::Mode::BPP2] = new uint8[8192]();
  tilevalid[Background::Mode::BPP4] = new uint8[4096]();
  tilevalid[Background::Mode::BPP8] = new uint8[2048]();
}

PPU::Cache::~Cache() {
  delete[] tiledata[Background::Mode::BPP2];
  delete[] tiledata[Background::Mode::BPP4];
  delete[] tiledata[Background::Mode::BPP8];
  delete[] tilevalid[Background::Mode::BPP2];
  delete[] tilevalid[Background::Mode::BPP4];
  delete[] tilevalid[Background::Mode::BPP8];
}

auto PPU::Cache::tile2bpp(uint16 tile) -> uint8* {
  if(!tilevalid[0][tile]) {
    tilevalid[0][tile] = 1;
    uint8* output = (uint8*)tiledata[0] + (tile << 6);
    uint offset = tile << 3;
    uint y = 8;
    uint8 color;
    uint16 d0;
    while(y--) {
      d0 = ppu.vram[offset +  0];
      #define render_line(index) \
        color  = d0.bit(index + 0) << 0; \
        color |= d0.bit(index + 8) << 1; \
        *output++ = color
      render_line(7);
      render_line(6);
      render_line(5);
      render_line(4);
      render_line(3);
      render_line(2);
      render_line(1);
      render_line(0);
      #undef render_line
      offset++;
    }
  }

  return tiledata[0] + (tile << 6);
}

auto PPU::Cache::tile4bpp(uint16 tile) -> uint8* {
  if(!tilevalid[1][tile]) {
    tilevalid[1][tile] = 1;
    uint8* output = (uint8*)tiledata[1] + (tile << 6);
    uint offset = tile << 4;
    uint y = 8;
    uint8 color;
    uint16 d0, d1;
    while(y--) {
      d0 = ppu.vram[offset +  0];
      d1 = ppu.vram[offset +  8];
      #define render_line(index) \
        color  = d0.bit(index + 0) << 0; \
        color |= d0.bit(index + 8) << 1; \
        color |= d1.bit(index + 0) << 2; \
        color |= d1.bit(index + 8) << 3; \
        *output++ = color
      render_line(7);
      render_line(6);
      render_line(5);
      render_line(4);
      render_line(3);
      render_line(2);
      render_line(1);
      render_line(0);
      #undef render_line
      offset++;
    }
  }

  return tiledata[1] + (tile << 6);
}

auto PPU::Cache::tile8bpp(uint16 tile) -> uint8* {
  if(!tilevalid[2][tile]) {
    tilevalid[2][tile] = 1;
    uint8* output = (uint8*)tiledata[2] + (tile << 6);
    uint offset = tile << 5;
    uint y = 8;
    uint8 color;
    uint16 d0, d1, d2, d3;
    while(y--) {
      d0 = ppu.vram[offset +  0];
      d1 = ppu.vram[offset +  8];
      d2 = ppu.vram[offset + 16];
      d3 = ppu.vram[offset + 24];
      #define render_line(index) \
        color  = d0.bit(index + 0) << 0; \
        color |= d0.bit(index + 8) << 1; \
        color |= d1.bit(index + 0) << 2; \
        color |= d1.bit(index + 8) << 3; \
        color |= d2.bit(index + 0) << 4; \
        color |= d2.bit(index + 8) << 5; \
        color |= d3.bit(index + 0) << 6; \
        color |= d3.bit(index + 8) << 7; \
        *output++ = color
      render_line(7);
      render_line(6);
      render_line(5);
      render_line(4);
      render_line(3);
      render_line(2);
      render_line(1);
      render_line(0);
      #undef render_line
      offset++;
    }
  }

  return tiledata[2] + (tile << 6);
}

auto PPU::Cache::tile(uint bpp, uint16 tile) -> uint8* {
  switch(bpp) {
  case Background::Mode::BPP2: return tile2bpp(tile);
  case Background::Mode::BPP4: return tile4bpp(tile);
  case Background::Mode::BPP8: return tile8bpp(tile);
  }
}

//marks all tiledata cache entries as dirty
auto PPU::Cache::flush() -> void {
  for(uint i : range(8192)) tilevalid[Background::Mode::BPP2][i] = 0;
  for(uint i : range(4096)) tilevalid[Background::Mode::BPP4][i] = 0;
  for(uint i : range(2048)) tilevalid[Background::Mode::BPP8][i] = 0;
}
