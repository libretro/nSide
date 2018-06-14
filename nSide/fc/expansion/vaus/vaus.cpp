VausE::VausE() {
  create(Expansion::Enter, system.frequency());
  latched = 0;
  counter = 0;

  x = 0;
  control = 0;

  prev = 0;
}

auto VausE::main() -> void {
  uint next = ppu.vcounter() * 341 + ppu.hcounter();

  if(next < prev) {
    //Vcounter wrapped back to zero; update control knob for start of new frame
    int nx = platform->inputPoll(ID::Port::Expansion, ID::Device::VausE, Control) * 160 / 256;
    const uint8_t trimpot = 0x0d;
    x = max(trimpot, min(trimpot + 0xa0, x - nx));
  }

  prev = next;
  step(3);
  synchronize(cpu);
}

auto VausE::data1() -> bool {
  return platform->inputPoll(ID::Port::Expansion, ID::Device::VausE, Fire);
}

auto VausE::data2() -> uint5 {
  if(latched == 1) return control.bit(7) << 1;
  if(counter >= 8) return 0;

  return control.bit(7 - counter++) << 1;
}

auto VausE::write(uint3 data) -> void {
  if(latched == data.bit(0)) return;
  latched = data.bit(0);
  counter = 0;

  control = x & 0xff;
}
