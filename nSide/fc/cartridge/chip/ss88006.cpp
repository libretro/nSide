struct SS88006 : Chip {
  SS88006(Board& board, Markup::Node& boardNode) : Chip(board) {
  }

  auto main() -> void {
    if(irqEnable) {
      irqCounter = (irqCounter & ~irqMask) | ((irqCounter - 1) & irqMask);
      if((irqCounter & irqMask) == irqMask) cpu.irqLine(1);
    }
    tick();
  }

  auto prgAddress(uint addr) const -> uint {
    switch((addr >> 13) & 3) {
    case 0: return (prgBank[0] << 13) | (addr & 0x1fff);
    case 1: return (prgBank[1] << 13) | (addr & 0x1fff);
    case 2: return (prgBank[2] << 13) | (addr & 0x1fff);
    case 3: return (0xff << 13) | (addr & 0x1fff);
    }
  }

  auto chrAddress(uint addr) const -> uint {
    return (chrBank[addr >> 10] << 10) | (addr & 0x03ff);
  }

  auto ciramAddress(uint addr) const -> uint {
    switch(mirror) {
    case 0: return (addr & 0x03ff) | ((addr & 0x0800) >> 1);
    case 1: return (addr & 0x03ff) | ((addr & 0x0400) >> 0);
    case 2: return (addr & 0x03ff) | 0x0000;
    case 3: return (addr & 0x03ff) | 0x0400;
    }
  }

  auto ramRead(uint addr, uint8 data) -> uint8 {
    if(ramEnable) return board.read(board.prgram, addr & 0x1fff);
  }

  auto ramWrite(uint addr, uint8 data) -> void {
    if(ramEnable && ramWriteEnable) board.write(board.prgram, addr & 0x1fff, data);
  }

  auto regWrite(uint addr, uint8 data) -> void {
    data &= 0x0f;
    switch(addr & 0xf003) {
    case 0x8000: prgBank[0] = (prgBank[0] & 0xf0) | (data << 0); break;
    case 0x8001: prgBank[0] = (prgBank[0] & 0x0f) | (data << 4); break;
    case 0x8002: prgBank[1] = (prgBank[1] & 0xf0) | (data << 0); break;
    case 0x8003: prgBank[1] = (prgBank[1] & 0x0f) | (data << 4); break;
    case 0x9000: prgBank[2] = (prgBank[2] & 0xf0) | (data << 0); break;
    case 0x9001: prgBank[2] = (prgBank[2] & 0x0f) | (data << 4); break;
    case 0x9002:
      ramEnable = data & 0x01;
      ramWriteEnable = data & 0x02;
      break;
    case 0xa000: chrBank[0] = (chrBank[0] & 0xf0) | (data << 0); break;
    case 0xa001: chrBank[0] = (chrBank[0] & 0x0f) | (data << 4); break;
    case 0xa002: chrBank[1] = (chrBank[1] & 0xf0) | (data << 0); break;
    case 0xa003: chrBank[1] = (chrBank[1] & 0x0f) | (data << 4); break;
    case 0xb000: chrBank[2] = (chrBank[2] & 0xf0) | (data << 0); break;
    case 0xb001: chrBank[2] = (chrBank[2] & 0x0f) | (data << 4); break;
    case 0xb002: chrBank[3] = (chrBank[3] & 0xf0) | (data << 0); break;
    case 0xb003: chrBank[3] = (chrBank[3] & 0x0f) | (data << 4); break;
    case 0xc000: chrBank[4] = (chrBank[4] & 0xf0) | (data << 0); break;
    case 0xc001: chrBank[4] = (chrBank[4] & 0x0f) | (data << 4); break;
    case 0xc002: chrBank[5] = (chrBank[5] & 0xf0) | (data << 0); break;
    case 0xc003: chrBank[5] = (chrBank[5] & 0x0f) | (data << 4); break;
    case 0xd000: chrBank[6] = (chrBank[6] & 0xf0) | (data << 0); break;
    case 0xd001: chrBank[6] = (chrBank[6] & 0x0f) | (data << 4); break;
    case 0xd002: chrBank[7] = (chrBank[7] & 0xf0) | (data << 0); break;
    case 0xd003: chrBank[7] = (chrBank[7] & 0x0f) | (data << 4); break;
    case 0xe000: irqLatch = (irqLatch & 0xfff0) | (data << 0x0); break;
    case 0xe001: irqLatch = (irqLatch & 0xff0f) | (data << 0x4); break;
    case 0xe002: irqLatch = (irqLatch & 0xf0ff) | (data << 0x8); break;
    case 0xe003: irqLatch = (irqLatch & 0x0fff) | (data << 0xc); break;
    case 0xf000:
      cpu.irqLine(0);
      irqCounter = irqLatch;
      break;
    case 0xf001:
      cpu.irqLine(0);
      irqEnable = data & 0x01;
      irqMask = data & 0x08 ? 0x000f : (data & 0x04 ? 0x00ff : (data & 0x02 ? 0x0fff : 0xffff));
      break;
    case 0xf002: mirror = data & 0x03; break;
    case 0xf003: break; //TODO: μPD7756C ADPCM sound IC
    }
  }

  auto power(bool reset) -> void {
    prgBank[0] = 0;
    prgBank[1] = 0;
    prgBank[2] = 0;
    chrBank[0] = 0;
    chrBank[1] = 0;
    chrBank[2] = 0;
    chrBank[3] = 0;
    chrBank[4] = 0;
    chrBank[5] = 0;
    chrBank[6] = 0;
    chrBank[7] = 0;
    mirror = 0;
    ramEnable = 0;
    ramWriteEnable = 0;
    irqCounter = 0;
    irqLatch = 0;
    irqEnable = false;
    irqMask = 0xffff;
  }

  auto serialize(serializer& s) -> void {
    s.array(prgBank);
    s.array(chrBank);
    s.integer(mirror);
    s.integer(ramEnable);
    s.integer(ramWriteEnable);
    s.integer(irqCounter);
    s.integer(irqLatch);
    s.integer(irqEnable);
    s.integer(irqMask);
  }

  uint8 prgBank[3];
  uint8 chrBank[8];
  uint2 mirror;
  bool ramEnable;
  bool ramWriteEnable;
  uint16 irqCounter;
  uint16 irqLatch;
  bool irqEnable;
  uint16 irqMask;
};
