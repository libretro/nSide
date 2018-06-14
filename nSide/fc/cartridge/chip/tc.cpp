struct TC : Chip {
  TC(Board& board, Markup::Node& boardNode) : Chip(board) {
    auto chip = boardNode.find("chip");
    string type = chip(0)["type"].text();
    if(type == "TC0190FMC") revision = Revision::TC0190FMC;
    if(type == "TC0350FMR") revision = Revision::TC0350FMR;
    if(type == "TC0690FMR") revision = Revision::TC0690FMR;
    settings.pal16r4 = chip(1)["type"].text() == "PAL16R4";
  }

  auto main() -> void {
    if(revision != Revision::TC0190FMC) {
      if(irqDelay) irqDelay--;
      irqTest(ppu.io.chrAddressBus);
      cpu.irqLine(irqLine);
    }
    tick();
  }

  auto irqTest(uint addr) -> void {
    bool edge;
    //IRQs occur a little after they would on the MMC3.
    //Do they use the MC-ACC's behavior instead?
    edge = (chrAddressBus & 0x1000) && !(addr & 0x1000);
    if(edge) {
      if(irqDelay == 0) {
        if(irqCounter == 0xff) {
          irqCounter = irqLatch;
        } else if(++irqCounter == 0xff) {
          if(irqEnable) irqLine = 1;
        }
      }
      irqDelay = 6;
    }
    chrAddressBus = addr;
  }

  auto prgAddress(uint addr) const -> uint {
    switch((addr >> 13) & 3) {
    case 0: return (prgBank[0] << 13) | (addr & 0x1fff);
    case 1: return (prgBank[1] << 13) | (addr & 0x1fff);
    case 2: return (0x3e << 13) | (addr & 0x1fff);
    case 3: return (0x3f << 13) | (addr & 0x1fff);
    }
  }

  auto chrAddress(uint addr) const -> uint {
    if(addr <= 0x07ff) return (chrBank[0] << 11) | (addr & 0x07ff);
    if(addr <= 0x0fff) return (chrBank[1] << 11) | (addr & 0x07ff);
    if(addr <= 0x13ff) return (chrBank[2] << 10) | (addr & 0x03ff);
    if(addr <= 0x17ff) return (chrBank[3] << 10) | (addr & 0x03ff);
    if(addr <= 0x1bff) return (chrBank[4] << 10) | (addr & 0x03ff);
    if(addr <= 0x1fff) return (chrBank[5] << 10) | (addr & 0x03ff);
  }

  auto ciramAddress(uint addr) const -> uint {
    if(mirror == 0) return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
    if(mirror == 1) return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
  }

  auto regWrite(uint addr, uint8 data) -> void {
    switch(addr & 0xe003) {
    case 0x8000:
      prgBank[0] = data & 0x3f;
      if(revision != Revision::TC0690FMR && !settings.pal16r4) mirror = data & 0x40;
      break;
    case 0x8001:
      prgBank[1] = data & 0x3f;
      break;

    case 0x8002: chrBank[0] = data; break;
    case 0x8003: chrBank[1] = data; break;
    case 0xa000: chrBank[2] = data; break;
    case 0xa001: chrBank[3] = data; break;
    case 0xa002: chrBank[4] = data; break;
    case 0xa003: chrBank[5] = data; break;

    case 0xc000:
      if(revision != Revision::TC0190FMC) break;
      irqLatch = data;
      break;

    case 0xc001:
      if(revision != Revision::TC0190FMC) break;
      irqCounter = 0xff;
      break;

    case 0xc002:
      if(revision != Revision::TC0190FMC) break;
      irqEnable = true;
      break;

    case 0xc003:
      if(revision != Revision::TC0190FMC) break;
      irqEnable = false;
      irqLine = 0;
      break;

    case 0xe000:
      if(revision == Revision::TC0690FMR || settings.pal16r4) mirror = data & 0x40;
      break;
    }
  }

  auto power(bool reset) -> void {
    prgBank[0] = 0;
    prgBank[1] = 0;
    chrBank[0] = 0;
    chrBank[1] = 0;
    chrBank[2] = 0;
    chrBank[3] = 0;
    chrBank[4] = 0;
    chrBank[5] = 0;
    mirror = 0;
    irqLatch = 0;
    irqCounter = 0;
    irqEnable = false;
    irqDelay = 0;
    irqLine = 0;

    chrAddressBus = 0;
  }

  auto serialize(serializer& s) -> void {
    s.array(prgBank);
    s.array(chrBank);
    s.integer(mirror);
    s.integer(irqLatch);
    s.integer(irqCounter);
    s.integer(irqEnable);
    s.integer(irqDelay);
    s.integer(irqLine);

    s.integer(chrAddressBus);
  }

  enum class Revision : uint {
    TC0190FMC,  //No IRQ
    TC0350FMR,  //identical to TC0190 except with IRQ that no game uses
    TC0690FMR,
  } revision;

  struct Settings {
    bool pal16r4;
  } settings;

  uint8 prgBank[2];
  uint8 chrBank[6];
  bool mirror;
  uint8 irqLatch;
  uint8 irqCounter;
  bool irqEnable;
  uint irqDelay;
  bool irqLine;

  uint16 chrAddressBus;
};
