auto CPU::serialize(serializer& s) -> void {
  MOS6502::serialize(s);
  Thread::serialize(s);

  s.integer(io.rdyLine);
}
