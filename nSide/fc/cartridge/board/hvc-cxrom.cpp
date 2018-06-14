//HVC-CNROM
//NES-CPROM

struct HVC_CxROM : Board {
  HVC_CxROM(Markup::Node& boardNode) : Board(boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*CNROM" )) revision = Revision::CNROM;
    if(type.match("*CPROM" )) revision = Revision::CPROM;
    if(type.match("*PT-554")) revision = Revision::CNROM;
    if(type == "TENGEN-800008") revision = Revision::CNROM;

    settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
    settings.security = (bool)boardNode["security"];
    if(settings.security) {
      settings.pass = boardNode["security/pass"].natural();
    }
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, addr);
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      data &= readPRG(addr, data);  //Bus conflicts
      chrBank = data & 0x03;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    if(settings.security) {
      if(chrBank != settings.pass) return data;
    }
    switch(revision) {
    case Revision::CNROM: {
      addr = (chrBank * 0x2000) + (addr & 0x1fff);
      break;
    }

    case Revision::CPROM: {
      if(addr < 0x1000) addr = addr & 0x1fff;
      else              addr = (chrBank * 0x1000) + (addr & 0x0fff);
      break;
    }

    }

    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }

    switch(revision) {
    case Revision::CNROM: {
      addr = (chrBank * 0x2000) + (addr & 0x1fff);
      break;
    }

    case Revision::CPROM: {
      if(addr < 0x1000) addr = addr & 0x1fff;
      else              addr = (chrBank * 0x1000) + (addr & 0x0fff);
      break;
    }

    }
    Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    chrBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(chrBank);
  }

  enum class Revision : uint {
    CNROM,
    CPROM,
  } revision;

  struct Settings {
    bool mirror;  //0 = vertical, 1 = horizontal
    bool security;
    uint2 pass;
  } settings;

  uint2 chrBank;
};
