#pragma once

//Atari 2600 emulator by hex_usr, with contributions from:
// byuu        (MOS6502 processor)
// Alyosha_TAS (PIA TIA, timing details)
//license: GPLv3
//original project started: 2016-07-31

#include <emulator/emulator.hpp>
#include <emulator/thread.hpp>
#include <emulator/scheduler.hpp>
#include <emulator/random.hpp>
#include <emulator/cheat.hpp>

#include <processor/mos6502/mos6502.hpp>

namespace Atari2600 {
  #define platform Emulator::platform
  namespace File = Emulator::File;
  using Scheduler = Emulator::Scheduler;
  using Random = Emulator::Random;
  using Cheat = Emulator::Cheat;
  extern Scheduler scheduler;
  extern Random random;
  extern Cheat cheat;

  struct Thread : Emulator::Thread {
    auto create(auto (*entrypoint)() -> void, double frequency) -> void {
      Emulator::Thread::create(entrypoint, frequency);
      scheduler.append(*this);
    }

    inline auto synchronize(Thread& thread) -> void {
      if(clock() >= thread.clock()) scheduler.resume(thread);
    }
  };

  struct Region {
    static inline auto NTSC() -> bool;
    static inline auto PAL() -> bool;
    static inline auto SECAM() -> bool;
  };

  #include <a2600/controller/controller.hpp>

  #include <a2600/cpu/cpu.hpp>
  #include <a2600/pia/pia.hpp>
  #include <a2600/tia/tia.hpp>

  #include <a2600/system/system.hpp>
  #include <a2600/cartridge/cartridge.hpp>
}

#include <a2600/interface/interface.hpp>
