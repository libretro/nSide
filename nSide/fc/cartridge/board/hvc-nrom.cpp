//HVC-FAMILYBASIC
//HVC-HROM
//HVC-NROM-128
//HVC-NROM-256
//HVC-RROM
//HVC-RROM-128
//HVC-RTROM
//HVC-SROM
//HVC-STROM
//NAMCOT-3301
//NAMCOT-3302
//NAMCOT-3303
//NAMCOT-3304
//NAMCOT-3305
//NAMCOT-3311
//NAMCOT-3312

struct HVC_NROM : Board {
  HVC_NROM(Markup::Node& boardNode) : Board(boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*FAMILYBASIC*")) revision = Revision::FAMILYBASIC;
    if(type.match("*HROM*"       )) revision = Revision::HROM;
    if(type.match("*NROM*"       )) revision = Revision::NROM;
    if(type.match("*RROM*"       )) revision = Revision::RROM;
    if(type.match("*RTROM*"      )) revision = Revision::RTROM;
    if(type.match("*SROM*"       )) revision = Revision::SROM;
    if(type.match("*STROM*"      )) revision = Revision::STROM;
    if(type.match("NAMCOT-33??"  )) revision = Revision::Namco33xx;
    if(revision == Revision::HROM) {
      settings.mirror = 0;
    } else {
      settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
    }
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x8000) return read(prgrom, addr);
    if(revision == Revision::FAMILYBASIC && (addr & 0xe000) == 0x6000) return read(prgram, addr);
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(revision == Revision::FAMILYBASIC && (addr & 0xe000) == 0x6000) write(prgram, addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr & 0x07ff);
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

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
  }

  enum class Revision : uint {
    FAMILYBASIC,
    HROM,
    NROM,
    RROM,
    RTROM,
    SROM,
    STROM,
    Namco33xx,
  } revision;

  struct Settings {
    bool mirror;  //0 = vertical, 1 = horizontal
  } settings;
};
