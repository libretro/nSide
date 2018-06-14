#include "SPC_DSP.h"

struct DSP {
  shared_pointer<Emulator::Stream> stream;
  uint8 apuram[64 * 1024];

  DSP();

  alwaysinline auto step(uint clocks) -> void;

  auto mute() -> bool;
  auto read(uint8 addr) -> uint8;
  auto write(uint8 addr, uint8 data) -> void;

  auto main() -> void;
  auto load(Markup::Node) -> bool;
  auto power(bool reset) -> void;

  auto serialize(serializer&) -> void;

  int64 clock;

private:
  SPC_DSP spc_dsp;
  int16_t samplebuffer[8192];
};

extern DSP dsp;
