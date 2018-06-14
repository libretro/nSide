#include <a2600/a2600.hpp>

namespace Atari2600 {

CPU cpu;
#include "memory.cpp"
#include "timing.cpp"
#include "serialization.cpp"

CPU::CPU() : Processor::MOS6502() {
}

auto CPU::Enter() -> void {
  while(true) scheduler.synchronize(), cpu.main();
}

auto CPU::main() -> void {
  instruction();
}

auto CPU::load(Markup::Node node) -> bool {
  return true;
}

auto CPU::power() -> void {
  //CPU's clock rate is the same in all regions
  create(Enter, Emulator::Constants::Colorburst::NTSC);
  MOS6502::BCD = 1;
  MOS6502::power();
  MOS6502::reset();

  //CPU
  r.pc  = cartridge.access(0xfffc, r.mdr) << 0;
  r.pc |= cartridge.access(0xfffd, r.mdr) << 8;

  io.rdyLine = 1;
}

}
