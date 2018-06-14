FourPlayers::FourPlayers() {
  latched = 0;
  counter1 = 0;
  counter2 = 0;
}

auto FourPlayers::data1() -> bool {
  if(counter1 >= 24) return 1;
  if(latched) return platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, 0 + A);

  auto& A = gamepads[0];
  auto& B = gamepads[2];

  switch(counter1++) {
  case  0: return A.a;
  case  1: return A.b;
  case  2: return A.select;
  case  3: return A.start;
  case  4: return A.up & !A.down;
  case  5: return A.down & !A.up;
  case  6: return A.left & !A.right;
  case  7: return A.right & !A.left;
  case  8: return B.a;
  case  9: return B.b;
  case 10: return B.select;
  case 11: return B.start;
  case 12: return B.up & !B.down;
  case 13: return B.down & !B.up;
  case 14: return B.left & !B.right;
  case 15: return B.right & !B.left;
  case 16: return 0;
  case 17: return 0;
  case 18: return 1;
  case 19: return 0;
  case 20: return 0;
  case 21: return 0;
  case 22: return 0;
  case 23: return 0;
  }
  unreachable;
}

auto FourPlayers::data2() -> uint5 {
  if(counter2 >= 24) return 2;
  if(latched) return platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, 8 + A) << 1;

  auto& A = gamepads[1];
  auto& B = gamepads[3];

  switch(counter2++) {
  case  0: return A.a << 1;
  case  1: return A.b << 1;
  case  2: return A.select << 1;
  case  3: return A.start << 1;
  case  4: return (A.up & !A.down) << 1;
  case  5: return (A.down & !A.up) << 1;
  case  6: return (A.left & !A.right) << 1;
  case  7: return (A.right & !A.left) << 1;
  case  8: return B.a << 1;
  case  9: return B.b << 1;
  case 10: return B.select << 1;
  case 11: return B.start << 1;
  case 12: return (B.up & !B.down) << 1;
  case 13: return (B.down & !B.up) << 1;
  case 14: return (B.left & !B.right) << 1;
  case 15: return (B.right & !B.left) << 1;
  case 16: return 0;
  case 17: return 0;
  case 18: return 0;
  case 19: return 2;
  case 20: return 0;
  case 21: return 0;
  case 22: return 0;
  case 23: return 0;
  }
  unreachable;
}

auto FourPlayers::write(uint3 data) -> void {
  if(latched == data.bit(0)) return;
  latched = data.bit(0);
  counter1 = 0;
  counter2 = 0;

  if(latched == 0) {
    for(uint id : range(4)) {
      auto& gamepad = gamepads[id];
      gamepad.a      = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + A);
      gamepad.b      = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + B);
      gamepad.select = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + Select);
      gamepad.start  = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + Start);
      gamepad.up     = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + Up);
      gamepad.down   = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + Down);
      gamepad.left   = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + Left);
      gamepad.right  = platform->inputPoll(ID::Port::Expansion, ID::Device::FourPlayers, id * 8 + Right);
    }
  }
}
