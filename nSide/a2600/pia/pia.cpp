#include <a2600/a2600.hpp>

namespace Atari2600 {

PIA pia;

#include "memory.cpp"
#include "io.cpp"
#include "serialization.cpp"

PIA::PIA() {
}

PIA::~PIA() {
}

auto PIA::step(uint clocks) -> void {
  Thread::step(clocks);
  for(auto peripheral : peripherals) synchronize(*peripheral);
  synchronize(cpu);
}

auto PIA::Enter() -> void {
  while(true) scheduler.synchronize(), pia.main();
}

auto PIA::main() -> void {
  runTimer();
  step(3);

  //Behavior not relevant to the Atari 2600:
  //The MOS 6507 has no IRQ pin, so IRQ functionality is left unused.
  //This means that timers and PA7 (left controller port pressing Right)
  //cannot automatically fire IRQs.
}

auto PIA::load(Markup::Node node) -> bool {
  return true;
}

auto PIA::power() -> void {
  //PIA's clock rate is the same in all regions
  create(Enter, Emulator::Constants::Colorburst::NTSC);

  random.array(ram, sizeof(ram));

  io.tvtype = TVType::Color;
  io.difficulty0 = Difficulty::B;
  io.difficulty1 = Difficulty::B;

  io.swcha  = 0x00;
  io.swacnt = 0x00;
  io.swchb  = 0x00;
  io.swbcnt = 0x00;

  io.timer.value          = 0x00000;
  io.timerDecrement       = 1024;
  io.timerUnderflowINSTAT = false;
  io.timerUnderflowTIM_T  = false;

  io.timerIRQEnable = false;
  io.pa7IRQEnable   = false;
  io.pa7EdgeDetect  = false;
}

auto PIA::updateIO() -> void {
  //If this code runs 19912 (262 * 228 / 3) times as fast as it does in other
  //emulators, it will tremendously slow down emulation.
  //Therefore, a dirty hack is used where this function is called from within
  //the TIA.
  if(platform->inputPoll(ID::Port::Hardware, ID::Device::Controls, 2)) {
    if(!input.tvtype) io.tvtype = !io.tvtype;
    input.tvtype = 1;
  } else {
    input.tvtype = 0;
  }

  if(platform->inputPoll(ID::Port::Hardware, ID::Device::Controls, 3)) {
    if(!input.difficulty0) io.difficulty0 = !io.difficulty0;
    input.difficulty0 = 1;
  } else {
    input.difficulty0 = 0;
  }

  if(platform->inputPoll(ID::Port::Hardware, ID::Device::Controls, 4)) {
    if(!input.difficulty1) io.difficulty1 = !io.difficulty1;
    input.difficulty1 = 1;
  } else {
    input.difficulty1 = 0;
  }
}

auto PIA::runTimer() -> void {
  io.timer.value = (io.timer.value - io.timerDecrement) & 0x3ffff;
  if(io.timer.value == 0) {
    io.timerUnderflowINSTAT = true;
    io.timerUnderflowTIM_T  = true;
    io.timerDecrement = 1024;
  }
}

}
