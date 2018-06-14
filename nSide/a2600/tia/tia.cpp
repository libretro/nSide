#include <a2600/a2600.hpp>

namespace Atari2600 {

TIA tia;
#include "io.cpp"
#include "render.cpp"
#include "audio.cpp"
#include "serialization.cpp"

TIA::TIA() {
  output = new uint32[160 * 228];
}

TIA::~TIA() {
  delete[] output;
}

auto TIA::step(uint clocks) -> void {
  while(clocks--) {
    if(++io.hcounter >= 228) {
      io.hcounter = 0;
      if(io.vcounter < 511) io.vcounter++;
      //Safe-guard against software that does not use VSYNC properly
      if(io.vcounter >= (Region::NTSC() ? 262 : 312)) io.vcounter = 0;
    }
    Thread::step(1);
    synchronize(cpu);
  }
}

auto TIA::Enter() -> void {
  while(true) scheduler.synchronize(), tia.main();
}

auto TIA::main() -> void {
  scanline();

  io.hcounter = 0;
  for(uint x : range(57)) {
    if(x == 0) cpu.rdyLine(1);
    for(uint sub : range(4)) {
      runVideo();
      runAudio();
      step(1);
    }
  }
}

auto TIA::load(Markup::Node node) -> bool {
  return true;
}

auto TIA::power() -> void {
  create(Enter, system.frequency());
  stream = Emulator::audio.createStream(1, frequency() / 114.0);
  memory::fill(output, 160 * 228 * sizeof(uint32));

  io.playerMissile0Color   = 0x00;
  io.playerMissile1Color   = 0x00;
  io.playfieldBallColor    = 0x00;
  io.backgroundColor       = 0x00;

  io.playfieldBallPriority = false;

  player[0].numberSize = 0;
  player[0].reflect    = 0;
  player[0].graphic    = 0;
  player[0].position   = 0;
  player[0].motion     = 0;

  player[1].numberSize = 0;
  player[1].reflect    = 0;
  player[1].graphic    = 0;
  player[1].position   = 0;
  player[1].motion     = 0;

  missile[0].enable   = false;
  missile[0].size     = 0;
  missile[0].position = 0;
  missile[0].motion   = 0;

  missile[1].enable   = false;
  missile[1].size     = 0;
  missile[1].position = 0;
  missile[1].motion   = 0;

  ball.enable   = false;
  ball.size     = 0;
  ball.position = 0;
  ball.motion   = 0;

  playfield.reflect = false;
  playfield.score   = false;

  playfield.graphic0 = 0x00;
  playfield.graphic1 = 0x00;
  playfield.graphic2 = 0x00;

  collision.m0p0 = false;
  collision.m0p1 = false;
  collision.m1p0 = false;
  collision.m1p1 = false;
  collision.p0bl = false;
  collision.p0pf = false;
  collision.p1bl = false;
  collision.p1pf = false;
  collision.m0bl = false;
  collision.m0pf = false;
  collision.m1bl = false;
  collision.m1pf = false;
  collision.blpf = false;
  collision.m0m1 = false;
  collision.p0p1 = false;

  for(auto& channel : audio.channel) {
    channel.control = 0;
    channel.frequency = 0;
    channel.volume = 0;

    channel.phase = 0;
    channel.state1 = 1;
    channel.state3 = 2;
    channel.shift4 = ~0;
    channel.shift5 = ~0;
    channel.shift9 = ~0;
    channel.sample = 1;
  }
}

auto TIA::refresh() -> void {
  auto output = this->output;
  auto pitch = 160;
  auto width = 160;
  auto height = 228;
  Emulator::video.refresh(output, pitch * sizeof(uint32), width, height);
}

}
