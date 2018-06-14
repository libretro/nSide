auto CPU::step(uint clocks) -> void {
  Thread::step(clocks);
  for(auto peripheral : peripherals) synchronize(*peripheral);

  synchronize(apu);
  synchronize(ppu);
  synchronize(cartridgeSlot[bus.slot]);
  for(auto coprocessor : coprocessors) synchronize(*coprocessor);
}

auto CPU::lastCycle() -> void {
  io.interruptPending = ((io.irqLine | io.apuLine) & ~r.p.i) | io.nmiPending;
}

auto CPU::nmi(uint16& vector) -> void {
  if(io.nmiPending) {
    io.nmiPending = false;
    vector = 0xfffa;
  }
}

auto CPU::oamdma() -> void {
  for(uint n = 0; n < 256; n++) {
    uint8 data = read((io.oamdmaPage << 8) + n);
    write(0x2004, data);
  }
}

auto CPU::nmiLine(bool line) -> void {
  //edge-sensitive (0->1)
  if(!io.nmiLine && line) {
    io.nmiPending = true;
    if(Model::PlayChoice10()) playchoice10.nmiDetected = true;
  }
  io.nmiLine = line;
}

auto CPU::irqLine(bool line) -> void {
  //level-sensitive
  io.irqLine = line;
}

auto CPU::apuLine(bool line) -> void {
  //level-sensitive
  io.apuLine = line;
}

auto CPU::rdyLine(bool line) -> void {
  io.rdyLine = line;
}

auto CPU::rdyAddr(bool valid, uint16 value) -> void {
  io.rdyAddrValid = valid;
  io.rdyAddrValue = value;
}
