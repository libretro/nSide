JoyPair::JoyPair() {
  latched = 0;
  counter1 = 0;
  counter2 = 0;
}

auto JoyPair::data1() -> bool {
  if(counter1 >= 8) return 1;
  if(latched) return platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, 0 + A);

  auto& A = gamepads[0];

  switch(counter1++) {
  case 0: return A.a;
  case 1: return A.b;
  case 2: return A.select;
  case 3: return A.start;
  case 4: return A.up & !A.down;
  case 5: return A.down & !A.up;
  case 6: return A.left & !A.right;
  case 7: return A.right & !A.left;
  }
  unreachable;
}

auto JoyPair::data2() -> uint5 {
  if(counter2 >= 8) return 2;
  if(latched) return platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, 8 + A) << 1;

  auto& B = gamepads[1];

  switch(counter2++) {
  case 0: return B.a << 1;
  case 1: return B.b << 1;
  case 2: return B.select << 1;
  case 3: return B.start << 1;
  case 4: return (B.up & !B.down) << 1;
  case 5: return (B.down & !B.up) << 1;
  case 6: return (B.left & !B.right) << 1;
  case 7: return (B.right & !B.left) << 1;
  }
  unreachable;
}

auto JoyPair::write(uint3 data) -> void {
  if(latched == data.bit(0)) return;
  latched = data.bit(0);
  counter1 = 0;
  counter2 = 0;

  if(latched == 0) {
    for(uint id : range(2)) {
      auto& gamepad = gamepads[id];
      gamepad.a      = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + A);
      gamepad.b      = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + B);
      gamepad.select = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + Select);
      gamepad.start  = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + Start);
      gamepad.up     = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + Up);
      gamepad.down   = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + Down);
      gamepad.left   = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + Left);
      gamepad.right  = platform->inputPoll(ID::Port::Expansion, ID::Device::JoyPair, id * 8 + Right);
    }
  }
}
