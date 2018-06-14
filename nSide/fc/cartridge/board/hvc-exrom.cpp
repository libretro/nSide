struct HVC_ExROM : Board {
  HVC_ExROM(Markup::Node& boardNode) : Board(boardNode), mmc5(*this, boardNode) {
    chip = &mmc5;
    string type = boardNode["id"].text();
    if(type.match("*EKROM*")) revision = Revision::EKROM;
    if(type.match("*ELROM*")) revision = Revision::ELROM;
    if(type.match("*ETROM*")) revision = Revision::ETROM;
    if(type.match("*EWROM*")) revision = Revision::EWROM;
  }

  auto main() -> void {
    mmc5.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    return mmc5.readPRG(addr, data);
  }

  auto writePRG(uint addr, uint8 data) -> void {
    mmc5.writePRG(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    return mmc5.readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    mmc5.writeCHR(addr, data);
  }

  auto scanline(uint y) -> void {
    mmc5.scanline(y);
  }

  auto power(bool reset) -> void {
    mmc5.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    mmc5.serialize(s);
  }

  enum class Revision : uint {
    EKROM,
    ELROM,
    ETROM,
    EWROM,
  } revision;

  MMC5 mmc5;
};
