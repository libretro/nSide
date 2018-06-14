struct IFH3001 : Chip {
  IFH3001(Board& board, Markup::Node& boardNode) : Chip(board) {
  }

  auto main() -> void {
    if(irqEnable && irqCounter) {
      if(--irqCounter == 0) cpu.irqLine(1);
    }
    tick();
  }

  auto prgAddress(uint addr) const -> uint {
    switch(addr & 0xe000) {
    case 0x8000: return (prgBank[0] << 13) | (addr & 0x1fff);
    case 0xa000: return (prgBank[1] << 13) | (addr & 0x1fff);
    case 0xc000: return (prgBank[2] << 13) | (addr & 0x1fff);
    case 0xe000: return (0xff << 13) | (addr & 0x1fff);
    }
  }

  auto chrAddress(uint addr) const -> uint {
    return (chrBank[addr >> 10] << 10) | (addr & 0x03ff);
  }

  auto ciramAddress(uint addr) const -> uint {
    switch(mirror) {
    case 0: return (addr & 0x03ff) | ((addr & 0x0400) >> 0);
    case 1: return (addr & 0x03ff) | ((addr & 0x0800) >> 1);
    }
  }

  auto regWrite(uint addr, uint8 data) -> void {
    switch(addr & 0xf007) {
    case 0x8000:
    case 0x8001:
    case 0x8002:
    case 0x8003:
    case 0x8004:
    case 0x8005:
    case 0x8006:
    case 0x8007: prgBank[0] = data; break;
    case 0x9001: mirror = data & 0x80; break;
    case 0x9003:
      irqEnable = data & 0x80;
      cpu.irqLine(0);
      break;
    case 0x9004:
      irqCounter = irqLatch;
      cpu.irqLine(0);
      break;
    case 0x9005: irqLatch = (irqLatch & 0x00ff) | (data << 8); break;
    case 0x9006: irqLatch = (irqLatch & 0xff00) | (data << 0); break;
    case 0xa000:
    case 0xa001:
    case 0xa002:
    case 0xa003:
    case 0xa004:
    case 0xa005:
    case 0xa006:
    case 0xa007: prgBank[1] = data; break;
    case 0xb000: chrBank[0] = data; break;
    case 0xb001: chrBank[1] = data; break;
    case 0xb002: chrBank[2] = data; break;
    case 0xb003: chrBank[3] = data; break;
    case 0xb004: chrBank[4] = data; break;
    case 0xb005: chrBank[5] = data; break;
    case 0xb006: chrBank[6] = data; break;
    case 0xb007: chrBank[7] = data; break;
    case 0xc000:
    case 0xc001:
    case 0xc002:
    case 0xc003:
    case 0xc004:
    case 0xc005:
    case 0xc006:
    case 0xc007: prgBank[2] = data; break;
    }
  }

  auto power(bool reset) -> void {
    prgBank[0] = 0x00;
    prgBank[1] = 0x01;
    prgBank[2] = 0xfe;
    chrBank[0] = 0;
    chrBank[1] = 0;
    chrBank[2] = 0;
    chrBank[3] = 0;
    chrBank[4] = 0;
    chrBank[5] = 0;
    chrBank[6] = 0;
    chrBank[7] = 0;
    mirror = 0;
    irqCounter = 0;
    irqLatch = 0;
    irqEnable = false;
  }

  auto serialize(serializer& s) -> void {
    s.array(prgBank);
    s.array(chrBank);
    s.integer(mirror);
    s.integer(irqCounter);
    s.integer(irqLatch);
    s.integer(irqEnable);
  }

  bool prgMode;
  uint8 prgBank[3];
  uint8 chrBank[8];
  bool mirror;
  uint16 irqCounter;
  uint16 irqLatch;
  bool irqEnable;
};
