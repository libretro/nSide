#include <fc/fc.hpp>

namespace Famicom {

APU apuM(0);
APU apuS(1);

#define bus (side ? busS : busM)

#include "envelope.cpp"
#include "sweep.cpp"
#include "pulse.cpp"
#include "triangle.cpp"
#include "noise.cpp"
#include "dmc.cpp"
#include "serialization.cpp"

#define cpu (side ? cpuS : cpuM)

APU::APU(bool side) : side(side), noise(*this), dmc(*this) {
  for(uint amp : range(32)) {
    if(amp == 0) {
      pulseDAC[amp] = 0;
    } else {
      pulseDAC[amp] = 16384.0 * 95.88 / (8128.0 / amp + 100.0);
    }
  }

  for(uint dmc_amp : range(128)) {
    for(uint triangle_amp : range(16)) {
      for(uint noise_amp : range(16)) {
        if(dmc_amp == 0 && triangle_amp == 0 && noise_amp == 0) {
          dmcTriangleNoiseDAC[dmc_amp][triangle_amp][noise_amp] = 0;
        } else {
          dmcTriangleNoiseDAC[dmc_amp][triangle_amp][noise_amp]
          = 16384.0 * 159.79 / (100.0 + 1.0 / (triangle_amp / 8227.0 + noise_amp / 12241.0 + dmc_amp / 22638.0));
        }
      }
    }
  }
}

auto APU::Enter() -> void {
  while(true) {
    scheduler.synchronize();
    if(apuM.active()) apuM.main();
    if(apuS.active()) apuS.main();
  }
}

auto APU::main() -> void {
  uint pulse_output, triangle_output, noise_output, dmc_output;

  pulse_output  = pulse[0].clock();
  pulse_output += pulse[1].clock();
  triangle_output = triangle.clock();
  noise_output = noise.clock();
  dmc_output = dmc.clock();

  clockFrameCounterDivider();

  int output = 0;
  output += pulseDAC[pulse_output];
  output += dmcTriangleNoiseDAC[dmc_output][triangle_output][noise_output];
//output  = filter.runHipassStrong(output);
  output += cartridgeSample;
//output  = filter.runHipassWeak(output);
//output  = filter.runLopass(output);
  if(stream) {
    if(!Model::PlayChoice10() || playchoice10.apuOutput) {
      stream->sample(sclamp<16>(output) / 32768.0);
    } else {
      stream->sample(0.0);
    }
  }

  tick();
}

auto APU::tick() -> void {
  Thread::step(rate());
  synchronize(cpu);
}

auto APU::setIRQ() -> void {
  cpu.apuLine(frame.irqPending || dmc.irqPending);
}

auto APU::setSample(int16 sample) -> void {
  cartridgeSample = sample;
}

auto APU::load(Markup::Node node) -> bool {
  if(Model::VSSystem()) return true;

  string versionString;
  if(Model::Famicom()) {
    if(Region::NTSCJ() || Region::NTSCU()) versionString = node["apu/ntsc-version"].text();
    if(Region::PAL()) versionString = node["apu/pal-version"].text();
    if(Region::Dendy()) versionString = node["apu/dendy-version"].text();
  } else {
    versionString = node["apu/version"].text();
  }

  //NTSC
  if(versionString == "RP2A03")         version = Version::RP2A03;
  if(versionString == "RP2A03A")        version = Version::RP2A03A;
  if(versionString == "RP2A03C")        version = Version::RP2A03C;
  if(versionString == "RP2A03E")        version = Version::RP2A03E;
  if(versionString == "RP2A03F")        version = Version::RP2A03F;
  if(versionString == "RP2A03G")        version = Version::RP2A03G;
  if(versionString == "RP2A03H")        version = Version::RP2A03H;

  //PAL
  if(versionString == "RP2A07G")        version = Version::RP2A07G;

  //Dendy
  if(versionString == "TA-03NP1-6527P") version = Version::TA_03NP1_6527P;
  if(versionString == "UA6527P")        version = Version::UA6527P;

  return true;
}

auto APU::power(bool reset) -> void {
  create(APU::Enter, system.frequency());
  if(Model::Famicom()
  || Model::VSSystem() && (vssystem.gameCount != 2 || side == 0)
  || Model::PlayChoice10() && !reset
  || Model::FamicomBox() && !reset
  ) {
    stream = Emulator::audio.createStream(1, system.frequency() / rate());
    stream->addFilter(Emulator::Filter::Order::First, Emulator::Filter::Type::HighPass, 90.0);
    stream->addFilter(Emulator::Filter::Order::First, Emulator::Filter::Type::HighPass, 440.0);
    stream->addFilter(Emulator::Filter::Order::First, Emulator::Filter::Type::LowPass, 14000.0);
    stream->addFilter(Emulator::Filter::Order::Second, Emulator::Filter::Type::LowPass, 20000.0, 3);
  }

  pulse[0].power(reset);
  pulse[1].power(reset);
  triangle.power(reset);
  noise.power(reset);
  dmc.power(reset);

  frame.irqPending = 0;

  frame.mode = 0;
  frame.counter = 0;
  frame.divider = 1;

  enabledChannels = 0;
  cartridgeSample = 0;

  setIRQ();
}

auto APU::readIO(uint16 addr, uint8 data) -> uint8 {
  switch(addr) {

  case 0x4015: {
    data.bit(0) = pulse[0].lengthCounter > 0;
    data.bit(1) = pulse[1].lengthCounter > 0;
    data.bit(2) = triangle.lengthCounter > 0;
    data.bit(3) =    noise.lengthCounter > 0;
    data.bit(4) =      dmc.lengthCounter > 0;
    data.bit(5) = 0;
    data.bit(6) =       frame.irqPending;
    data.bit(7) =         dmc.irqPending;

    frame.irqPending = false;
    setIRQ();
  }

  }

  return data;
}

auto APU::writeIO(uint16 addr, uint8 data) -> void {
  const uint n = (addr >> 2) & 1;  //pulse#

  switch(addr) {

  case 0x4000: case 0x4004: {
    pulse[n].duty = data >> 6;
    pulse[n].envelope.loopMode = data & 0x20;
    pulse[n].envelope.useSpeedAsVolume = data & 0x10;
    pulse[n].envelope.speed = data & 0x0f;
    if(version == APU::Version::UA6527P) {
      pulse[n].duty = pulse[n].duty.bit(0) << 1 | pulse[n].duty.bit(1) << 0;
    }
    return;
  }

  case 0x4001: case 0x4005: {
    pulse[n].sweep.enable = data & 0x80;
    pulse[n].sweep.period = (data & 0x70) >> 4;
    pulse[n].sweep.decrement = data & 0x08;
    pulse[n].sweep.shift = data & 0x07;
    pulse[n].sweep.reload = true;
    return;
  }

  case 0x4002: case 0x4006: {
    pulse[n].period = (pulse[n].period & 0x0700) | (data << 0);
    pulse[n].sweep.pulsePeriod = (pulse[n].sweep.pulsePeriod & 0x0700) | (data << 0);
    return;
  }

  case 0x4003: case 0x4007: {
    pulse[n].period = (pulse[n].period & 0x00ff) | (data << 8);
    pulse[n].sweep.pulsePeriod = (pulse[n].sweep.pulsePeriod & 0x00ff) | (data << 8);

    pulse[n].dutyCounter = 0;
    pulse[n].envelope.reloadDecay = true;

    if(enabledChannels & (1 << n)) {
      pulse[n].lengthCounter = lengthCounterTable[(data >> 3) & 0x1f];
    }
    return;
  }

  case 0x4008: {
    triangle.haltLengthCounter = data & 0x80;
    triangle.linearLength = data & 0x7f;
    return;
  }

  case 0x400a: {
    triangle.period = (triangle.period & 0x0700) | (data << 0);
    return;
  }

  case 0x400b: {
    triangle.period = (triangle.period & 0x00ff) | (data << 8);

    triangle.reloadLinear = true;

    if(enabledChannels & (1 << 2)) {
      triangle.lengthCounter = lengthCounterTable[(data >> 3) & 0x1f];
    }
    return;
  }

  case 0x400c: {
    noise.envelope.loopMode = data & 0x20;
    noise.envelope.useSpeedAsVolume = data & 0x10;
    noise.envelope.speed = data & 0x0f;
    return;
  }

  case 0x400e: {
    //TODO: Check if the RP2A03E and prior versions support short mode.
    //Currently assuming that the RP2A03F is bugged. See noise.cpp for bug implementation.
    if(version != Version::RP2A03) noise.shortMode = data.bit(7);
    noise.period = data.bits(0,3);
    return;
  }

  case 0x400f: {
    noise.envelope.reloadDecay = true;

    if(enabledChannels & (1 << 3)) {
      noise.lengthCounter = lengthCounterTable[(data >> 3) & 0x1f];
    }
    return;
  }

  case 0x4010: {
    dmc.irqEnable = data & 0x80;
    dmc.loopMode = data & 0x40;
    dmc.period = data & 0x0f;

    dmc.irqPending = dmc.irqPending && dmc.irqEnable && !dmc.loopMode;
    setIRQ();
    return;
  }

  case 0x4011: {
    dmc.dacLatch = data & 0x7f;
    return;
  }

  case 0x4012: {
    dmc.addrLatch = data;
    return;
  }

  case 0x4013: {
    dmc.lengthLatch = data;
    return;
  }

  case 0x4015: {
    if((data & 0x01) == 0) pulse[0].lengthCounter = 0;
    if((data & 0x02) == 0) pulse[1].lengthCounter = 0;
    if((data & 0x04) == 0) triangle.lengthCounter = 0;
    if((data & 0x08) == 0)    noise.lengthCounter = 0;

    (data & 0x10) ? dmc.start() : dmc.stop();
    dmc.irqPending = false;

    setIRQ();
    enabledChannels = data & 0x1f;
    return;
  }

  case 0x4017: {
    frame.mode = data >> 6;

    frame.counter = 0;
    if(frame.mode & 2) clockFrameCounter();
    if(frame.mode & 1) {
      frame.irqPending = false;
      setIRQ();
    }
    frame.divider = !Region::PAL() ? FrameCounter::NtscPeriod : FrameCounter::PalPeriod;
    return;
  }

  }
}

auto APU::clockFrameCounter() -> void {
  frame.counter++;

  if(frame.counter & 1) {
    pulse[0].clockLength();
    pulse[0].sweep.clock(0);
    pulse[1].clockLength();
    pulse[1].sweep.clock(1);
    triangle.clockLength();
    noise.clockLength();
  }

  pulse[0].envelope.clock();
  pulse[1].envelope.clock();
  triangle.clockLinearLength();
  noise.envelope.clock();

  if(frame.counter == 0) {
    if(frame.mode & 2) {
      if(Region::NTSCJ()) frame.divider += FrameCounter::NtscPeriod;
      if(Region::NTSCU()) frame.divider += FrameCounter::NtscPeriod;
      if(Region::PAL  ()) frame.divider += FrameCounter::PalPeriod;
      if(Region::Dendy()) frame.divider += FrameCounter::PalPeriod;
    }
    if(frame.mode == 0) {
      frame.irqPending = true;
      setIRQ();
    }
  }
}

auto APU::clockFrameCounterDivider() -> void {
  frame.divider -= 2;
  if(frame.divider <= 0) {
    clockFrameCounter();
    if(Region::NTSCJ()) frame.divider += FrameCounter::NtscPeriod;
    if(Region::NTSCU()) frame.divider += FrameCounter::NtscPeriod;
    if(Region::PAL  ()) frame.divider += FrameCounter::PalPeriod;
    if(Region::Dendy()) frame.divider += FrameCounter::PalPeriod;
  }
}

const uint8 APU::lengthCounterTable[32] = {
  0x0a, 0xfe, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xa0, 0x08, 0x3c, 0x0a, 0x0e, 0x0c, 0x1a, 0x0e,
  0x0c, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xc0, 0x18, 0x48, 0x1a, 0x10, 0x1c, 0x20, 0x1e,
};

const uint16 APU::noisePeriodTableNTSC[16] = {
  4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

const uint16 APU::noisePeriodTablePAL[16] = {
  4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708,  944, 1890, 3778,
};

const uint16 APU::dmcPeriodTableNTSC[16] = {
  428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54,
};

const uint16 APU::dmcPeriodTablePAL[16] = {
  398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118,  98, 78, 66, 50,
};

#undef bus
#undef cpu

}
