struct FDS : Board {
  FDS(Markup::Node& boardNode) : Board(boardNode) {
  }

  auto main() -> void {
    if(irqEnable && irqCounter > 0 && --irqCounter == 0) {
      cpu.irqLine(1);
      if(irqRepeat) irqCounter = irqLatch;
      else           irqEnable = false;
      irqLatch = 0;  //for Kaettekita Mario Bros.?
    } else {
      //TODO: Drive timer emulation
    }
    tick();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    switch(addr) {
    case 0x4030: {
      data = (irqPending | (byteTransferred << 1));
      irqPending = false;
      byteTransferred = false;
      //0x10: CRC 0=pass; 1=fail
      //0x40: End of disk head
      //0x80: Disk read/write enable
      cpu.irqLine(0);
      return 0;
    }

    case 0x4031: {
      return dataRegister;
    }

    case 0x4032: {
      return (
        (diskSlotEmpty      << 0) |
        (diskNotReady       << 1) |
        (diskWriteProtected << 2)
      );
    }

    case 0x4033: {
      //7-bit expansion input; 0x80 means low battery
      return 0x00;
    }

    case 0x4090: {
      return data & 0xc0;
    }

    case 0x4092: {
      return data & 0xc0;
    }

    }

    if((addr & 0xffc0) == 0x4040) {
      return wavetable[addr & 0x3f] | (data & 0xc0);
    }
    switch(addr & 0xe000) {
    case 0x6000:
    case 0x8000:
    case 0xa000:
    case 0xc000: return read(prgram, addr);
    case 0xe000: return read(prgrom, addr);
    }
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    switch(addr) {
    case 0x4020: irqLatch = (irqLatch & 0xff00) | (data << 0); break;
    case 0x4021: irqLatch = (irqLatch & 0x00ff) | (data << 8); break;
    case 0x4022: {
      irqRepeat = data & 0x01;
      irqEnable = data & 0x02;
      irqCounter = irqLatch;
      byteTransferred = false;
      cpu.irqLine(0);  //if pending IRQ flag is clear
      break;
    }

    case 0x4023: {
      diskIOEnable = data & 0x01;
      soundIOEnable = data & 0x02;
      break;
    }

    case 0x4024: {
      //clear pending IRQ flag
      if(!byteTransferred) cpu.irqLine(0);
      break;
    }

    case 0x4025: {
      fdsControl = data & 0xf7;
      mirror = data & 0x08;
      break;
    }

    case 0x4026: {
      //7-bit expansion output
      break;
    }

    case 0x4080: break;
    case 0x4082: break;
    case 0x4083: break;
    case 0x4084: break;
    case 0x4085: break;
    case 0x4086: break;
    case 0x4087: break;
    case 0x4088: break;
    case 0x4089: {
      wavetableWriteEnable = data & 0x80;
      break;
    }

    case 0x408a: break;

    }

    if((addr & 0xffc0) == 0x4040) {
      if(wavetableWriteEnable) wavetable[addr & 0x3f] = data & 0x3f;
    }

    switch(addr & 0xe000) {
    case 0x6000:
    case 0x8000:
    case 0xa000:
    case 0xc000: write(prgram, addr, data); break;
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(mirror) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(mirror) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    return Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    irqCounter = 0;
    irqLatch = 0;
    irqRepeat = false;
    irqEnable = false;

    diskIOEnable = false;
    soundIOEnable = false;

    fdsControl = 0x26;
    mirror = false;

    irqPending = false;
    byteTransferred = false;

    dataRegister = 0x00;

    diskSlotEmpty = true;
    diskNotReady = true;
    diskWriteProtected = true;

    for(auto& n : wavetable) n = 0x00;
    wavetableWriteEnable = false;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);

    s.integer(irqCounter);
    s.integer(irqLatch);
    s.integer(irqRepeat);
    s.integer(irqEnable);

    s.integer(diskIOEnable);
    s.integer(soundIOEnable);

    s.integer(fdsControl);
    s.integer(mirror);

    s.integer(irqPending);
    s.integer(byteTransferred);

    s.integer(dataRegister);

    s.integer(diskSlotEmpty);
    s.integer(diskNotReady);
    s.integer(diskWriteProtected);

    s.array(wavetable);
    s.integer(wavetableWriteEnable);
  }

  uint16 irqCounter;
  uint16 irqLatch;
  bool irqRepeat;
  bool irqEnable;

  bool diskIOEnable;
  bool soundIOEnable;

  uint8 fdsControl;
  bool mirror;

  bool irqPending;
  bool byteTransferred;

  uint8 dataRegister;

  bool diskSlotEmpty;
  bool diskNotReady;
  bool diskWriteProtected;

  uint6 wavetable[0x40];
  bool wavetableWriteEnable;

};
