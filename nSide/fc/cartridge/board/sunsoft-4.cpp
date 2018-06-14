//SUNSOFT-4

struct Sunsoft4 : Board {
  Sunsoft4(Markup::Node& boardNode) : Board(boardNode) {
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0xc000) == 0x8000) return read(prgrom, (prgBank << 14) | (addr & 0x3fff));
    if((addr & 0xc000) == 0xc000) return read(prgrom, (    0xff << 14) | (addr & 0x3fff));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    switch(addr & 0xf000) {
    case 0x8000: chrBank[0] = data; break;
    case 0x9000: chrBank[1] = data; break;
    case 0xa000: chrBank[2] = data; break;
    case 0xb000: chrBank[3] = data; break;
    case 0xc000: ntromBank[0] = data | 0x80; break;
    case 0xd000: ntromBank[1] = data | 0x80; break;
    case 0xe000:
      mirror = data & 0x03;
      nametableMode = data & 0x10;
      break;
    case 0xf000: prgBank = data; break;
    }
  }

  auto ciramAddress(uint addr) -> uint {
    switch(mirror) {
    case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
    case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    case 2: return 0x0000 | (addr & 0x03ff);
    case 3: return 0x0400 | (addr & 0x03ff);
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      addr = ciramAddress(addr);
      if(nametableMode) {
        return Board::readCHR(((ntromBank[(addr & 0x0400) >> 10] | 0x80) << 10) | (addr & 0x03FF), data);
      } else {
        return ppu.readCIRAM(addr);
      }
    }
    addr = (chrBank[(addr & 0x1800) >> 11] << 11) | (addr & 0x07ff);
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(nametableMode == 0) return ppu.writeCIRAM(ciramAddress(addr), data);
    } else {
      return Board::writeCHR(addr, data);
    }
  }

  auto power(bool reset) -> void {
    chrBank[0] = 0;
    chrBank[1] = 0;
    chrBank[2] = 0;
    chrBank[3] = 0;
    ntromBank[0] = 0;
    ntromBank[1] = 0;
    mirror = 0;
    nametableMode = 0;
    prgBank = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.array(chrBank);
    s.array(ntromBank);
    s.integer(mirror);
    s.integer(nametableMode);
    s.integer(prgBank);
  }

  uint8 chrBank[4];
  uint7 ntromBank[2];
  uint2 mirror;
  bool nametableMode;
  uint8 prgBank;
};
