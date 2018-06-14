SegaTap::SegaTap(uint port) : Controller(port) {
  for(uint4& data : buffer) data = 0xf;
  tr = 1;
  th = 1;
}

auto SegaTap::readData() -> uint7 {
  uint4 data;

  //Controller signatures:
  //0xf: None
  //0x0: Control Pad (2 nybbles)
  //0x1: Fighting Pad 6B (3 nybbles)
  //0x2: Mouse (6 nybbles)
  switch(counter) {
  case  0: data = 0b0011; break;  //Signature
  case  1: data = 0b1111; break;  //Signature
  case  2: data = 0b0000; break;  //Signature
  case  3: data = 0b0000; break;  //Signature
  case  4: data = 0x1; break;
  case  5: data = 0x1; break;
  case  6: data = 0x1; break;
  case  7: data = 0x1; break;
  default: data = counter < 32 ? buffer[counter - 8] : uint4(0xf); break;
  }

  return th << 6 | tr << 5 | tr << 4 | data;
}

auto SegaTap::writeData(uint7 data) -> void {
  if(!th && data.bit(6)) {
    counter = 0;
    uint index = 0;
    for(uint portID : range(4)) {
      buffer[index++] = (
        !platform->inputPoll(port, ID::Device::SegaTap, Up    + portID * 12) << 0
      | !platform->inputPoll(port, ID::Device::SegaTap, Down  + portID * 12) << 1
      | !platform->inputPoll(port, ID::Device::SegaTap, Left  + portID * 12) << 2
      | !platform->inputPoll(port, ID::Device::SegaTap, Right + portID * 12) << 3
      );
      buffer[index++] = (
        !platform->inputPoll(port, ID::Device::SegaTap, B     + portID * 12) << 0
      | !platform->inputPoll(port, ID::Device::SegaTap, C     + portID * 12) << 1
      | !platform->inputPoll(port, ID::Device::SegaTap, A     + portID * 12) << 2
      | !platform->inputPoll(port, ID::Device::SegaTap, Start + portID * 12) << 3
      );
      buffer[index++] = (
        !platform->inputPoll(port, ID::Device::SegaTap, Z     + portID * 12) << 0
      | !platform->inputPoll(port, ID::Device::SegaTap, Y     + portID * 12) << 1
      | !platform->inputPoll(port, ID::Device::SegaTap, X     + portID * 12) << 2
      | !platform->inputPoll(port, ID::Device::SegaTap, Mode  + portID * 12) << 3
      );
    }
    while(index < 24) buffer[index++] = 0xf;
  } else if((th << 6 | tr << 5) ^ (data & 0x60)) {
    if(!counter.bit(5)) counter++;
  }
  tr     = data.bit(5);
  th     = data.bit(6);
}
