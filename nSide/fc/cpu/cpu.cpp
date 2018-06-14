#include <fc/fc.hpp>

namespace Famicom {

CPU cpuM(0);
CPU cpuS(1);

#define bus (side ? busS : busM)
#define apu (side ? apuS : apuM)
#define ppu (side ? ppuS : ppuM)
#define controllerPort1 (side ? controllerPortS1 : controllerPortM1)
#define controllerPort2 (side ? controllerPortS2 : controllerPortM2)

#include "memory.cpp"
#include "io.cpp"
#include "timing.cpp"
#include "serialization.cpp"

CPU::CPU(bool side) : Processor::MOS6502(), side(side) {
}

auto CPU::Enter() -> void {
  while(true) {
    scheduler.synchronize();
    if(cpuM.active()) cpuM.main();
    if(cpuS.active()) cpuS.main();
  }
}

auto CPU::main() -> void {
  if(io.interruptPending) return interrupt();
  instruction();
}

auto CPU::load(Markup::Node node) -> bool {
  return true;
}

auto CPU::power(bool reset) -> void {
  if(!reset) {
    MOS6502::BCD = 0;
    MOS6502::power();
    coprocessors.reset();

    function<auto (uint16, uint8) -> uint8> reader;
    function<auto (uint16, uint8) -> void> writer;

    reader = [&](uint16 addr, uint8) -> uint8 { return ram[addr]; };
    writer = [&](uint16 addr, uint8 data) -> void { ram[addr] = data; };
    bus.map(reader, writer, "0000-1fff", 0x800);

    reader = {&CPU::readCPU, this};
    writer = {&CPU::writeCPU, this};
    bus.map(reader, writer, "4000-4017");

    random.array(ram, sizeof(ram));
  }

  create(Enter, system.frequency());
  MOS6502::reset();

  //CPU
  r.pc  = bus.read(0xfffc, r.mdr) << 0;
  r.pc |= bus.read(0xfffd, r.mdr) << 8;

  io.interruptPending = false;
  io.nmiPending = false;
  io.nmiLine = 0;
  io.irqLine = 0;
  io.apuLine = 0;

  io.rdyLine = 1;
  io.rdyAddrValid = false;
  io.rdyAddrValue = 0x0000;

  io.oamdmaPending = false;
  io.oamdmaPage = 0x00;
}

#undef bus
#undef apu
#undef ppu
#undef controllerPort1
#undef controllerPort2

}
