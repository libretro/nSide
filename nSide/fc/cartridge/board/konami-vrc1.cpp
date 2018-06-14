struct KonamiVRC1 : Board {
  KonamiVRC1(Markup::Node& boardNode) : Board(boardNode), vrc1(*this) {
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, vrc1.prgAddress(addr));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) return vrc1.regWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(vrc1.ciramAddress(addr));
    return Board::readCHR(vrc1.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(vrc1.ciramAddress(addr), data);
    return Board::writeCHR(vrc1.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    vrc1.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    vrc1.serialize(s);
  }

  VRC1 vrc1;
};
