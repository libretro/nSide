struct TaitoTC : Board {
  TaitoTC(Markup::Node& boardNode) : Board(boardNode), tc(*this, boardNode) {
  }

  auto main() -> void {
    tc.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, tc.prgAddress(addr));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) return tc.regWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(tc.ciramAddress(addr));
    return Board::readCHR(tc.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(tc.ciramAddress(addr), data);
    return Board::writeCHR(tc.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    tc.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    tc.serialize(s);
  }

  TC tc;
};
