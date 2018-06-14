//IREM-TAM-S1

struct IremTamS1 : Board {
  IremTamS1(Markup::Node& boardNode) : Board(boardNode) {
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x0000) return data;
    switch(addr & 0xc000) {
    case 0x8000: return read(prgrom, (   0x0f << 14) | (addr & 0x3fff));
    case 0xc000: return read(prgrom, (prgBank << 14) | (addr & 0x3fff));
    }
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      //TODO: check for bus conflicts
      //data &= readPRG(addr, data);
      prgBank = data & 0x0f;
      mirror = (data & 0xc0) >> 6;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      switch(mirror) {
      case 0: addr = (0x0000              ) | (addr & 0x03ff); break;
      case 1: addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff); break;
      case 2: addr = ((addr & 0x0400) >> 0) | (addr & 0x03ff); break;
      case 3: addr = (0x0400              ) | (addr & 0x03ff); break;
      }
      return ppu.readCIRAM(addr);
    }
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      switch(mirror) {
      case 0: addr = (0x0000              ) | (addr & 0x03ff); break;
      case 1: addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff); break;
      case 2: addr = ((addr & 0x0400) >> 0) | (addr & 0x03ff); break;
      case 3: addr = (0x0400              ) | (addr & 0x03ff); break;
      }
      return ppu.writeCIRAM(addr, data);
    }
    return Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    prgBank = 0;
    mirror = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);

    s.integer(prgBank);
    s.integer(mirror);
  }

  uint4 prgBank;
  uint2 mirror; //0 = screen 0, 1 = horizontal, 2 = vertical, 3 = screen 1
};
