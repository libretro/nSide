//JALECO-JF-24
//JALECO-JF-25

struct JalecoJF2x : Board {
  JalecoJF2x(Markup::Node& boardNode) : Board(boardNode), ss88006(*this, boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*JF-23")) revision = Revision::JF_23;
    if(type.match("*JF-24")) revision = Revision::JF_24;
    if(type.match("*JF-25")) revision = Revision::JF_25;
    if(type.match("*JF-27")) revision = Revision::JF_27;
    if(type.match("*JF-29")) revision = Revision::JF_29;
    if(type.match("*JF-37")) revision = Revision::JF_37;
    if(type.match("*JF-40")) revision = Revision::JF_40;
  }

  auto main() -> void {
    ss88006.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x8000) return read(prgrom, ss88006.prgAddress(addr));
    if((addr & 0xe000) == 0x6000) return ss88006.ramRead(addr, data);
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0x8000) == 0x8000) return ss88006.regWrite(addr, data);
    if((addr & 0xe000) == 0x6000) return ss88006.ramWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(ss88006.ciramAddress(addr));
    return Board::readCHR(ss88006.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(ss88006.ciramAddress(addr), data);
    return Board::writeCHR(ss88006.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    ss88006.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    ss88006.serialize(s);
  }

  enum class Revision : uint {
    JF_23,
    JF_24,
    JF_25,
    JF_27,
    JF_29,
    JF_37,
    JF_40,
  } revision;

  SS88006 ss88006;
};
