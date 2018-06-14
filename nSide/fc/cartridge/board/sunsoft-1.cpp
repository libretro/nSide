//SUNSOFT-1

struct Sunsoft1 : Board {
  Sunsoft1(Markup::Node& boardNode) : Board(boardNode) {
    settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, addr);
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0xe000) == 0x6000) {
      chrBank0 = data.bits(0,2);
      chrBank1 = data.bits(4,5);
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    switch(addr & 0x1000) {
    case 0x0000: return Board::readCHR((addr & 0xfff) | (chrBank0 << 12) | 0x0000, data);
    case 0x1000: return Board::readCHR((addr & 0xfff) | (chrBank1 << 12) | 0x4000, data);
    }
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    switch(addr & 0x1000) {
    case 0x0000: return Board::writeCHR((addr & 0xfff) | (chrBank0 << 12) | 0x0000, data);
    case 0x1000: return Board::writeCHR((addr & 0xfff) | (chrBank1 << 12) | 0x4000, data);
    }
  }

  auto power(bool reset) -> void {
    chrBank0 = 0;
    chrBank1 = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);

    s.integer(chrBank0);
    s.integer(chrBank1);
  }

  struct Settings {
    bool mirror;    //0 = vertical, 1 = horizontal
  } settings;

  uint3 chrBank0;
  uint2 chrBank1;
};
