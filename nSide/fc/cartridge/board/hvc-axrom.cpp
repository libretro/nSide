//HVC-AMROM
//NES-ANROM
//NES-AN1ROM
//HVC-AOROM

struct HVC_AxROM : Board {
  HVC_AxROM(Markup::Node& boardNode) : Board(boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*AMROM" )) revision = Revision::AMROM;
    if(type.match("*ANROM" )) revision = Revision::ANROM;
    if(type.match("*AN1ROM")) revision = Revision::AN1ROM;
    if(type.match("*AOROM" )) revision = Revision::AOROM;
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, (prgBank << 15) | (addr & 0x7fff));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      //Bus conflicts
      if(revision == Revision::AMROM) data &= readPRG(addr, data);
      prgBank = data & 0x0f;
      mirrorSelect = data & 0x10;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM((mirrorSelect << 10) | (addr & 0x03ff));
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM((mirrorSelect << 10) | (addr & 0x03ff), data);
    return Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    prgBank = 0x0f;
    mirrorSelect = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);

    s.integer(prgBank);
    s.integer(mirrorSelect);
  }

  enum class Revision : uint {
    AMROM,
    ANROM,
    AN1ROM,
    AOROM,
  } revision;

  uint4 prgBank;
  bool mirrorSelect;
};
