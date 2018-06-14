struct NES_HKROM : Board {
  NES_HKROM(Markup::Node& boardNode) : Board(boardNode), mmc6(*this) {
  }

  auto main() -> void {
    mmc6.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0xf000) == 0x7000) return mmc6.ramRead(addr, data);
    if(addr & 0x8000) return read(prgrom, mmc6.prgAddress(addr));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0xf000) == 0x7000) return mmc6.ramWrite(addr, data);
    if(addr & 0x8000) return mmc6.regWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    mmc6.irqTest(addr);
    if(addr & 0x2000) return ppu.readCIRAM(mmc6.ciramAddress(addr));
    return Board::readCHR(mmc6.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    mmc6.irqTest(addr);
    if(addr & 0x2000) return ppu.writeCIRAM(mmc6.ciramAddress(addr), data);
    return Board::writeCHR(mmc6.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    mmc6.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    mmc6.serialize(s);
  }

  MMC6 mmc6;
};
