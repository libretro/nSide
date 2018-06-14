//BANDAI-74*161/161/32

struct Bandai74_161_161_32 : Board {
  Bandai74_161_161_32(Markup::Node& boardNode) : Board(boardNode) {
    settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x0000) return data;
    if((addr & 0xc000) == 0x8000) return read(prgrom, (prgBank << 14) | (addr & 0x3fff));
    else                          return read(prgrom, (   0x0f << 14) | (addr & 0x3fff));
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      //TODO: check for bus conflicts
      data &= readPRG(addr, data);
      prgBank = (data & 0xf0) >> 4;
      chrBank = (data & 0x0f) >> 0;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    prgBank = 0;
    chrBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.integer(chrBank);
  }

  struct Settings {
    bool mirror;  //0 = vertical, 1 = horizontal
  } settings;

  uint4 prgBank;
  uint4 chrBank;
};
