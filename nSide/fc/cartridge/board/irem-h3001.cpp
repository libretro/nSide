struct IremH3001 : Board {
  IremH3001(Markup::Node& boardNode) : Board(boardNode), ifh3001(*this, boardNode) {
  }

  auto main() -> void {
    ifh3001.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x8000) return read(prgrom, ifh3001.prgAddress(addr));
    if((addr & 0xe000) == 0x6000) return read(prgram, addr & 0x1fff);
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0x8000) == 0x8000) return ifh3001.regWrite(addr, data);
    if((addr & 0xe000) == 0x6000) return write(prgram, addr & 0x1fff, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(ifh3001.ciramAddress(addr));
    return Board::readCHR(ifh3001.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(ifh3001.ciramAddress(addr), data);
    return Board::writeCHR(ifh3001.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    ifh3001.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    ifh3001.serialize(s);
  }

  IFH3001 ifh3001;
};
