#include <sfc-balanced/sfc.hpp>

namespace SuperFamicom {

DSP dsp;

#include "serialization.cpp"
#include "SPC_DSP.cpp"

DSP::DSP() {
}

auto DSP::step(uint clocks) -> void {
  clock += clocks;
}

auto DSP::main() -> void {
  spc_dsp.run(1);
  step(2);

  int count = spc_dsp.sample_count();
  if(count > 0) {
    for(uint n = 0; n < count; n += 2) stream->sample(samplebuffer[n + 0] / 32768.0, samplebuffer[n + 1] / 32768.0);
    spc_dsp.set_output(samplebuffer, 8192);
  }
}

auto DSP::mute() -> bool {
  return spc_dsp.mute();
}

auto DSP::read(uint8 addr) -> uint8 {
  return spc_dsp.read(addr);
}

auto DSP::write(uint8 addr, uint8 data) -> void {
  spc_dsp.write(addr, data);
}

auto DSP::load(Markup::Node node) -> bool {
  return true;
}

auto DSP::power(bool reset) -> void {
  if(!reset) {
    random.array(apuram, sizeof(apuram));

    spc_dsp.init(apuram);
    spc_dsp.reset();
  }

  clock = 0;
  stream = Emulator::audio.createStream(2, system.apuFrequency() / 768.0);

  spc_dsp.soft_reset();
  spc_dsp.set_output(samplebuffer, 8192);
}

}
