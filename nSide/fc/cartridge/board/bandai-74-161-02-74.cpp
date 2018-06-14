//BANDAI-74*161/02/74

struct Bandai74_161_02_74 : Board {
  Bandai74_161_02_74(Markup::Node& boardNode) : Board(boardNode) {
  }

  auto main() -> void {
    chrAddressBusTest(ppu.io.chrAddressBus);
    tick();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, (prgBank << 15) | (addr & 0x7fff));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      data &= readPRG(addr, data);
      prgBank = (data & 0x03) >> 0;
      chrPlane = (data & 0x04) >> 2;
    }
  }

  auto chrAddress(uint addr) -> uint {
    switch(addr & 0x1000) {
    case 0x0000: return (addr & 0x0fff) | (chrPlane << 14) | (chrBank << 12);
    case 0x1000: return (addr & 0x0fff) | (chrPlane << 14);
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    chrAddressBusTest(addr);
    if(addr & 0x2000) return ppu.readCIRAM(addr & 0x07ff);
    return Board::readCHR(chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    chrAddressBusTest(addr);
    if(addr & 0x2000) return ppu.writeCIRAM(addr & 0x07ff, data);
    Board::writeCHR(chrAddress(addr), data);
  }

  auto chrAddressBusTest(uint addr) -> void {
    if((addr & 0x3000) != (chrAddressBus & 0x3000) && (addr & 0x3000) == 0x2000) {
      //NOR logic reverses bank numbers
      chrBank = (((addr & 0x0300) ^ 0x0300) >> 8);
    }
    chrAddressBus = addr & 0x3000;
  }

  auto power(bool reset) -> void {
    prgBank = 0;
    chrPlane = 0;
    chrBank = 0;
    chrAddressBus = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.integer(chrPlane);
    s.integer(chrBank);
    s.integer(chrAddressBus);
  }

  uint2 prgBank;
  bool chrPlane;
  uint2 chrBank;
  uint16 chrAddressBus;
};
