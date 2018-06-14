//IREM-HOLYDIVER

struct IremHolyDiver : Board {
  IremHolyDiver(Markup::Node& boardNode) : Board(boardNode) {
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    switch(addr & 0xc000) {
    case 0x8000: return read(prgrom, (prgBank << 14) | (addr & 0x3fff));
    case 0xc000: return read(prgrom, (   0x07 << 14) | (addr & 0x3fff));
    }
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      //Bus conflicts
      data &= readPRG(addr, data);
      prgBank = (data & 0x07) >> 0;
      mirror = data & 0x08;
      chrBank = (data & 0xf0) >> 4;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    return Board::readCHR((chrBank * 0x2000) + (addr & 0x1fff), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    Board::writeCHR((chrBank * 0x2000) + (addr & 0x1fff), data);
  }

  auto power(bool reset) -> void {
    prgBank = 0;
    chrBank = 0;
    mirror = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.integer(chrBank);
    s.integer(mirror);
  }

  uint3 prgBank;
  uint4 chrBank;
  bool mirror;
};
