auto TIA::runAudio() -> void {
  if(io.hcounter != 0 && io.hcounter != 114) return;

  for(auto& channel : audio.channel) {
    if(channel.phase++ < channel.frequency) continue;
    channel.phase = 0;

    if(channel.control.bits(2,3) == 3) {
      if(++channel.state3 == 3) channel.state3 = 0;
      else continue;
    }

    switch(channel.control) {
    case 0x0:
    case 0xb:
      channel.sample = channel.shift4.bit(0);
      channel.shift4 = channel.shift4 >> 1 | 0x008;
      channel.shift5 = channel.shift5 >> 1 | 0x010;
      break;
    case 0x1:
      channel.sample = shift(channel.shift4);
      break;
    case 0x2:
      if((channel.shift5 & 0x00f) == 0x000) channel.sample = shift(channel.shift4);
      if((channel.shift5 & 0x00f) == 0x00f) channel.sample = shift(channel.shift4);
      shift(channel.shift5);
      break;
    case 0x3:
      if(shift(channel.shift5)) channel.sample = shift(channel.shift4);
      break;
    case 0x4:
      channel.shift4 = channel.shift4 >> 1 | 0x008;
      shift(channel.shift5);
      channel.sample = channel.state1++;
      break;
    case 0x5:
      channel.shift5 = channel.shift5 >> 1 | 0x010;
      shift(channel.shift4);
      channel.sample = channel.state1++;
      break;
    case 0x6:
    case 0xa:
    case 0xe:
      shift(channel.shift5);
      if((channel.shift5 & 0x00f) == 0x000) channel.sample = 0;
      if((channel.shift5 & 0x00f) == 0x00f) channel.sample = 1;
      break;
    case 0x7:
    case 0x9:
    case 0xf:
      channel.sample = shift(channel.shift5);
      break;
    case 0x8:
      channel.sample = shift(channel.shift9);
      break;
    case 0xc:
    case 0xd:
      channel.sample = channel.state1++;
      break;
    }
  }

  double sample = 0.0;
  sample += audio.channel[0].sample * audio.channel[0].volume * 32768.0 / 15.0 - 16384.0;
  sample += audio.channel[1].sample * audio.channel[1].volume * 32768.0 / 15.0 - 16384.0;

  //stream->sample(sample / 32768.0);
  stream->sample(0.0);
}

auto TIA::shift(uint4& shift) -> bool {
  bool result = shift.bit(0);
  shift = shift >> 1 | (shift.bit(1) ^ shift.bit(0)) << 3;
  return result;
}

auto TIA::shift(uint5& shift) -> bool {
  bool result = shift.bit(0);
  shift = shift >> 1 | (shift.bit(2) ^ shift.bit(0)) << 4;
  return result;
}

auto TIA::shift(uint9& shift) -> bool {
  bool result = shift.bit(0);
  shift = shift >> 1 | (shift.bit(4) ^ shift.bit(0)) << 8;
  return result;
}

