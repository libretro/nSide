//BANDAI-FCG
//BANDAI-FCG-1
//BANDAI-FCG-2
//BANDAI-JUMP2
//BANDAI-LZ93D50

struct BandaiFCG : Board {
  BandaiFCG(Markup::Node& boardNode) : Board(boardNode), fcg(*this, boardNode) {
    string type = boardNode["id"].text();
    revision = Revision::FCGAll;
    if(type.match("*FCG-1*"  )) revision = Revision::FCG1;
    if(type.match("*FCG-2*"  )) revision = Revision::FCG2;
    if(type.match("*JUMP2*"  )) revision = Revision::JUMP2;
    if(type.match("*LZ93D50*")) revision = Revision::LZ93D50;
  }

  auto main() -> void {
    fcg.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0xe000) == 0x6000) {
      switch(revision) {
      case Revision::LZ93D50:
        //TODO: serial EEPROM support
        return 0x00 | (data & 0xef);
      case Revision::JUMP2:
        return fcg.eepromI2C_SCL ? fcg.ramRead(addr, data) : data;
      }
    }
    if((addr & 0x8000) == 0x8000) {
      if(revision != Revision::JUMP2) {
        return read(prgrom, fcg.prgAddress(addr));
      } else {
        return read(prgrom, fcg.prgAddress(addr) | ((fcg.chrBank[(ppu.io.chrAddressBus >> 10) & 3] & 1) << 18));
      }
    }
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0xe000) == 0x6000) {
      switch(revision) {
      case Revision::FCGAll:
      case Revision::FCG1:
      case Revision::FCG2:
        return fcg.regWrite(addr, data);
      case Revision::LZ93D50:
        //TODO: serial EEPROM support
        break;
      case Revision::JUMP2:
        if(fcg.eepromI2C_SCL) return fcg.ramWrite(addr, data);
        else                   break;
      }
    }
    if((addr & 0x8000) == 0x8000) {
      switch(revision) {
      case Revision::FCGAll:
      case Revision::LZ93D50:
      case Revision::JUMP2:
        return fcg.regWrite(addr, data);
      }
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(fcg.ciramAddress(addr));
    if(chrrom.size()) return Board::readCHR(fcg.chrAddress(addr), data);
    if(chrram.size()) return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(fcg.ciramAddress(addr), data);
    if(chrram.size()) Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    fcg.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    fcg.serialize(s);
  }

  enum class Revision : uint {
    FCGAll,
    FCG1,
    FCG2,
    JUMP2,
    LZ93D50,
  } revision;

  FCG fcg;
};
