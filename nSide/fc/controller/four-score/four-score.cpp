FourScore::FourScore(bool side, uint port) : Controller(side, port) {
  latched = 0;
  counter = 0;
}

auto FourScore::data() -> uint3 {
  if(counter >= 24) return 1;
  if(latched) return platform->inputPoll(port, ID::Device::FourScore, 0 + A);

  auto& A = gamepads[0];
  auto& B = gamepads[1];

  switch(counter++) {
  case  0: return A.a;
  case  1: return A.b;
  case  2: return A.select;
  case  3: return A.start;
  case  4: return (A.up & !A.down);
  case  5: return (A.down & !A.up);
  case  6: return (A.left & !A.right);
  case  7: return (A.right & !A.left);
  case  8: return B.a;
  case  9: return B.b;
  case 10: return B.select;
  case 11: return B.start;
  case 12: return (B.up & !B.down);
  case 13: return (B.down & !B.up);
  case 14: return (B.left & !B.right);
  case 15: return (B.right & !B.left);
  case 16: return 0;
  case 17: return 0;
  case 18: return port == ID::Port::Controller2;
  case 19: return port == ID::Port::Controller1;
  case 20: return 0;
  case 21: return 0;
  case 22: return 0;
  case 23: return 0;
  }
  unreachable;
}

auto FourScore::latch(bool data) -> void {
  if(latched == data) return;
  latched = data;
  counter = 0;

  if(latched == 0) {
    for(uint id : range(2)) {
      auto& gamepad = gamepads[id];
      gamepad.a      = platform->inputPoll(port, ID::Device::FourScore, id * 8 + A);
      gamepad.b      = platform->inputPoll(port, ID::Device::FourScore, id * 8 + B);
      gamepad.select = platform->inputPoll(port, ID::Device::FourScore, id * 8 + Select);
      gamepad.start  = platform->inputPoll(port, ID::Device::FourScore, id * 8 + Start);
      gamepad.up     = platform->inputPoll(port, ID::Device::FourScore, id * 8 + Up);
      gamepad.down   = platform->inputPoll(port, ID::Device::FourScore, id * 8 + Down);
      gamepad.left   = platform->inputPoll(port, ID::Device::FourScore, id * 8 + Left);
      gamepad.right  = platform->inputPoll(port, ID::Device::FourScore, id * 8 + Right);
    }
  }
}
