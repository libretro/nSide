Gamepad::Gamepad(bool side, uint port) : Controller(side, port) {
  latched = 0;
  counter = 0;
}

auto Gamepad::data() -> uint3 {
  if(counter >= 8) return 1;
  if(latched == 1) return platform->inputPoll(port, ID::Device::Gamepad, A);

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

auto Gamepad::latch(bool data) -> void {
  if(latched == data) return;
  latched = data;
  counter = 0;

  if(latched == 0) {
    up     = platform->inputPoll(port, ID::Device::Gamepad, Up);
    down   = platform->inputPoll(port, ID::Device::Gamepad, Down);
    left   = platform->inputPoll(port, ID::Device::Gamepad, Left);
    right  = platform->inputPoll(port, ID::Device::Gamepad, Right);
    b      = platform->inputPoll(port, ID::Device::Gamepad, B);
    a      = platform->inputPoll(port, ID::Device::Gamepad, A);
    if(!Model::VSSystem()) {
      select = platform->inputPoll(port, ID::Device::Gamepad, Select);
      start  = platform->inputPoll(port, ID::Device::Gamepad, Start);
    } else {
      select = start = 0;
    }
  }
}
