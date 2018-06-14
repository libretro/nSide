PowerPad::PowerPad(bool side, uint port) : Controller(side, port) {
  latched = 0;
  counter = 0;

  b1 = b2  = b3  = b4  = 0;
  b5 = b6  = b7  = b8  = 0;
  b9 = b10 = b11 = b12 = 0;
}

auto PowerPad::data() -> uint3 {
  if(counter >= 8) return 0x6;
  if(latched == 1) return (platform->inputPoll(port, ID::Device::PowerPad, 1) << 1) | (platform->inputPoll(port, ID::Device::PowerPad, 3) << 2);

  switch(counter++) {
  case  0: return (b2  << 1) | (b4  << 2);
  case  1: return (b1  << 1) | (b3  << 2);
  case  2: return (b5  << 1) | (b12 << 2);
  case  3: return (b9  << 1) | (b8  << 2);
  case  4: return (b6  << 1) | (1   << 2);
  case  5: return (b10 << 1) | (1   << 2);
  case  6: return (b11 << 1) | (1   << 2);
  case  7: return (b7  << 1) | (1   << 2);
  }
}

auto PowerPad::latch(bool data) -> void {
  if(latched == data) return;
  latched = data;
  counter = 0;

  if(latched == 0) {
    auto id = ID::Device::PowerPad;
    b1  = platform->inputPoll(port, id,  0);
    b2  = platform->inputPoll(port, id,  1);
    b3  = platform->inputPoll(port, id,  2);
    b4  = platform->inputPoll(port, id,  3);
    b5  = platform->inputPoll(port, id,  4);
    b6  = platform->inputPoll(port, id,  5);
    b7  = platform->inputPoll(port, id,  6);
    b8  = platform->inputPoll(port, id,  7);
    b9  = platform->inputPoll(port, id,  8);
    b10 = platform->inputPoll(port, id,  9);
    b11 = platform->inputPoll(port, id, 10);
    b12 = platform->inputPoll(port, id, 11);
  }
}
