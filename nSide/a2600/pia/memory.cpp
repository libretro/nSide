auto PIA::readRAM(uint7 addr, uint8 data) -> uint8 {
  return ram[addr];
}

auto PIA::writeRAM(uint7 addr, uint8 data) -> void {
  ram[addr] = data;
}
