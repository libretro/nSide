auto PlayChoice10::serialize(serializer& s) -> void {
  pc10cpu.serialize(s);

  s.integer(dip);

  s.integer(nmiDetected);

  s.integer(vramAccess);
  s.integer(gameSelectStart);
  s.integer(ppuOutput);
  s.integer(apuOutput);
  s.integer(cpuReset);
  s.integer(cpuStop);
  s.integer(display);
  s.integer(z80NMI);
  s.integer(watchdog);
  s.integer(ppuReset);

  s.array(wram);
  s.array(sram);

  s.integer(channel);
  changeChannel(channel);
  s.integer(sramBank);
  s.integer(promTest);
  s.integer(promClock);
  s.integer(promAddress);

  s.integer(controller1GameSelect);
  s.integer(controller1Start);

  s.array(videoCircuit.vram);
}

auto PlayChoice10::CPU::serialize(serializer& s) -> void {
  Processor::Z80::serialize(s);
  Thread::serialize(s);

  s.integer(state.nmiLine);
  s.integer(state.intLine);
}
