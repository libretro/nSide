//IREM-74*161/161/21/138

struct Irem74_161_161_21_138 : Board {
  Irem74_161_161_21_138(Markup::Node& boardNode) : Board(boardNode) {
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, (prgBank << 15) | (addr & 0x7fff));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      //Bus conflicts
      data &= readPRG(addr, data);
      prgBank = (data & 0x0f) >> 0;
      chrBank = (data & 0xf0) >> 4;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ciramRead(addr);
    if((addr & 0x1800) == 0x0000) return read(chrrom, (addr & 0x07ff) | (chrBank << 11));
    return read(chrram, addr);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ciramWrite(addr, data);
    if((addr & 0x1800) != 0x0000) return write(chrram, addr & 0x1fff, data);
  }

  auto ciramRead(uint addr) -> uint8 {
    switch(addr & 0x0800) {
    case 0x0000: return read(chrram, addr & 0x1fff);
    case 0x0800: return ppu.readCIRAM(addr);
    }
  }

  auto ciramWrite(uint addr, uint8 data) -> void {
    switch(addr & 0x0800) {
    case 0x0000: return write(chrram, addr & 0x1fff, data);
    case 0x0800: return ppu.writeCIRAM(addr, data);
    }
  }

  auto power(bool reset) -> void {
    prgBank = 0;
    chrBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.integer(chrBank);
  }

  uint4 prgBank;
  uint4 chrBank;
};
