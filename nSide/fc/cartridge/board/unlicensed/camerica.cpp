struct Camerica : Board {
  Camerica(Markup::Node& boardNode) : Board(boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*ALGN*"  )) revision = Revision::ALGN;
    if(type.match("*ALGQ*"  )) revision = Revision::ALGQ;
    if(type.match("*BF9093*")) revision = Revision::BF9093;
    if(type.match("*BF9096*")) revision = Revision::BF9096;
    if(type.match("*BF9097*")) revision = Revision::BF9097;

    settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x0000) return data;
    if((addr & 0xc000) == 0x8000) {
      return read(prgrom, (prgBlock << 16) | (prgBank << 14) | (addr & 0x3fff));
    } else {
      if(revision != Revision::ALGQ && revision != Revision::BF9096) {
        return read(prgrom, (0x0f << 14) | (addr & 0x3fff));
      } else {
        return read(prgrom, (prgBlock << 16) | (0x03 << 14) | (addr & 0x3fff));
      }
    }
  }

  auto writePRG(uint addr, uint8 data) -> void {
    switch(revision) {
    case Revision::ALGQ:
    case Revision::BF9096:
      if((addr & 0xc000) == 0x8000) {
        prgBlock = (data & 0x18) >> 3;
      }
      break;
    case Revision::BF9097:
      if((addr & 0xe000) == 0x8000) {
        nametable = data & 0x10;
      }
      break;
    }
    if((addr & 0xc000) == 0xc000) {
      switch(revision) {
      case Revision::ALGN:
      case Revision::BF9093: prgBank = data & 0x0f; break;
      case Revision::ALGQ:
      case Revision::BF9096: prgBank = data & 0x03; break;
      case Revision::BF9097: prgBank = data & 0x07; break;
      }
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(ciramAddress(addr));
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(ciramAddress(addr), data);
    return Board::writeCHR(addr, data);
  }

  auto ciramAddress(uint addr) const -> uint {
    if(revision != Revision::BF9097) {
      switch(settings.mirror) {
      case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
      case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      }
    } else {
      return (addr & 0x03ff) | (nametable << 10);
    }
  }

  auto power(bool reset) -> void {
    prgBank = 0;
    prgBlock = 0;
    nametable = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);

    s.integer(prgBank);
    s.integer(prgBlock);
    s.integer(nametable);
  }

  enum class Revision : uint {
    ALGN,  //Aladdin Game Enhancer
    ALGQ,  //Aladdin Game Enhancer (Quattro Multicarts)
    BF9093,
    BF9096,
    BF9097,
  } revision;

  struct Settings {
    bool mirror;  //0 = vertical, 1 = horizontal
  } settings;

  uint4 prgBank;
  uint2 prgBlock;  //for ALGQ and BF9096
  bool nametable;  //for BF9097
};
