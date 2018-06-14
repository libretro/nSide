auto CPU::step(uint clocks) -> void {
  Thread::step(clocks);
  synchronize(pia);
  synchronize(tia);
}

auto CPU::lastCycle() -> void {
}

auto CPU::rdyLine(bool line) -> void {
  io.rdyLine = line;
}
