auto CPU::serialize(serializer& s) -> void {
  Z80::serialize(s);
  Z80::Bus::serialize(s);
  Thread::serialize(s);

  s.array(ram);

  s.integer(state.nmiLine);
  s.integer(state.intLine);

  s.integer(disable.io);
  s.integer(disable.bios);
  s.integer(disable.ram);
  s.integer(disable.mycard);
  s.integer(disable.cartridge);
  s.integer(disable.expansion);
}
