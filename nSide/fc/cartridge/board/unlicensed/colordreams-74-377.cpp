//COLORDREAMS-74*377

struct ColorDreams74_377 : Board {
  ColorDreams74_377(Markup::Node& boardNode) : Board(boardNode) {
    settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, (prgBank << 15) | (addr & 0x7fff));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      //Bus conflicts
      data &= readPRG(addr, data);
      //PRG and CHR bits are swapped relative to NES-GxROM.
      //Additionally, up to 16 CHR banks are available instead of 4.
      prgBank = (data & 0x03) >> 0;
      chrBank = (data & 0xf0) >> 4;
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

  uint2 prgBank;
  uint4 chrBank;
};
