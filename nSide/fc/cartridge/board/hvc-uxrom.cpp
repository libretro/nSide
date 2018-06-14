//HVC-UNROM
//HVC-UN1ROM
//HVC-UOROM

struct HVC_UxROM : Board {
  HVC_UxROM(Markup::Node& boardNode) : Board(boardNode) {
    settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
    string type = boardNode["id"].text();
    if(type.match("*UNROM" )) revision = Revision::UNROM;
    if(type.match("*UN1ROM")) revision = Revision::UN1ROM;
    if(type.match("*UOROM" )) revision = Revision::UOROM;
    type = boardNode["chip/type"].text();
    if(type.match("74*32")) chipType = ChipType::_7432;
    if(type.match("74*08")) chipType = ChipType::_7408;
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x0000) return data;
    switch(chipType) {
    case ChipType::_7432:
      switch(addr & 0xc000) {
      case 0x8000: return read(prgrom, (prgBank << 14) | (addr & 0x3fff));
      case 0xc000: return read(prgrom, (   0x0f << 14) | (addr & 0x3fff));
      }
    case ChipType::_7408:
      switch(addr & 0xc000) {
      case 0x8000: return read(prgrom, (   0x00 << 14) | (addr & 0x3fff));
      case 0xc000: return read(prgrom, (prgBank << 14) | (addr & 0x3fff));
      }
    }
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      //Bus conflicts
      data &= readPRG(addr, data);
      if(revision != Revision::UN1ROM) prgBank = data & 0x0f;
      if(revision == Revision::UN1ROM) prgBank = (data & 0x1c) >> 2;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    return Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    prgBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);

    s.integer(prgBank);
  }

  enum class Revision : uint {
    UNROM,
    UN1ROM,
    UOROM,
  } revision;

  enum class ChipType : uint {
    _7432,
    _7408,
  } chipType;

  struct Settings {
    bool mirror;    //0 = vertical, 1 = horizontal
  } settings;

  uint4 prgBank;
};
