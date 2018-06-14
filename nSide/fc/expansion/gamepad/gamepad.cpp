GamepadE::GamepadE() {
  latched = 0;
  counter = 0;
}

auto GamepadE::data1() -> bool {
  if(counter >= 8) return 1;
  if(latched == 1) return platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, A);

  //note: D-pad physically prevents up+down and left+right from being pressed at the same time
  switch(counter++) {
  case 0: return a;
  case 1: return b;
  case 2: return select;
  case 3: return start;
  case 4: return up & !down;
  case 5: return down & !up;
  case 6: return left & !right;
  case 7: return right & !left;
  }
  unreachable;
}

auto GamepadE::data2() -> uint5 {
  return 0;
}

auto GamepadE::write(uint3 data) -> void {
  if(latched == data.bit(0)) return;
  latched = data.bit(0);
  counter = 0;

  if(latched == 0) {
    a      = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, A);
    b      = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, B);
    select = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, Select);
    start  = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, Start);
    up     = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, Up);
    down   = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, Down);
    left   = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, Left);
    right  = platform->inputPoll(ID::Port::Expansion, ID::Device::GamepadE, Right);
  }
}
