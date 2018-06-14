struct NES_Event : Board {
  NES_Event(Markup::Node& boardNode) : Board(boardNode), mmc1(*this, boardNode) {
    dip = platform->dipSettings(BML::unserialize({
      "setting name=Time\n",
      "  option value=4 name=6:14.96\n",
      "  option value=0 name=4:59.97\n",
      "  option value=1 name=5:18.71\n",
      "  option value=2 name=5:37.46\n",
      "  option value=3 name=5:56.21\n",
      "  option value=5 name=6:33.71\n",
      "  option value=6 name=6:52.45\n",
      "  option value=7 name=7:11.20\n",
      "  option value=8 name=7:29.95\n",
      "  option value=9 name=7:48.70\n",
      "  option value=10 name=8:07.44\n",
      "  option value=11 name=8:26.19\n",
      "  option value=12 name=8:44.94\n",
      "  option value=13 name=9:03.69\n",
      "  option value=14 name=9:22.44\n",
      "  option value=15 name=9:41.18\n",
    }));
  }

  auto main() -> void {
    switch(prgLock) {
    case 0: if((mmc1.chrBank[0] & 0x10) == 0x10) prgLock++; break;
    case 1: if((mmc1.chrBank[0] & 0x10) == 0x00) prgLock++; break;
    case 2:
      if((mmc1.chrBank[0] & 0x10) == 0x00) irqCounter++;
      if((mmc1.chrBank[0] & 0x10) == 0x10) {
        irqCounter = 0x00000000;
        cpu.irqLine(0);
      }
      break;
    }
    if(irqCounter == (0x20000000 | (dip << 25))) cpu.irqLine(1);
    mmc1.main();
  }

  auto prgAddress(uint addr) -> uint {
    if(prgLock < 2) return addr & 0x7fff;
    switch(mmc1.chrBank[0] & 0x08) {
    case 0: return ((mmc1.chrBank[0] & 0x06) << 14) | (addr & 0x7fff);
    case 8: return mmc1.prgAddress(addr) | 0x20000;
    }
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0xe000) == 0x6000) {
      if(mmc1.ramDisable) return 0x00;
      if(prgram.size() > 0) return read(prgram, addr);
    }

    if(addr & 0x8000) {
      return read(prgrom, prgAddress(addr));
    }

    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0xe000) == 0x6000) {
      if(mmc1.ramDisable) return;
      if(prgram.size() > 0) return write(prgram, addr, data);
    }

    if(addr & 0x8000) return mmc1.mmioWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(mmc1.ciramAddress(addr));
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(mmc1.ciramAddress(addr), data);
    return Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    mmc1.power(reset);
    prgLock = 0;
    irqCounter = 0x00000000;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    mmc1.serialize(s);
    s.integer(prgLock);
    s.integer(irqCounter);
  }

  MMC1 mmc1;
  uint2 prgLock;
  uint30 irqCounter;
  uint4 dip;
};
