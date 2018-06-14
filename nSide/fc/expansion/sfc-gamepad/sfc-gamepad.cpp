SFCGamepad::SFCGamepad() {
  latched = 0;
  counter = 0;
}

auto SFCGamepad::data1() -> bool {
  if(counter >= 16) return 1;
  if(latched == 1) return platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, 0);

  //note: D-pad physically prevents up+down and left+right from being pressed at the same time
  switch(counter++) {
  case  0: return b;
  case  1: return y;
  case  2: return select;
  case  3: return start;
  case  4: return up & !down;
  case  5: return down & !up;
  case  6: return left & !right;
  case  7: return right & !left;
  case  8: return a;
  case  9: return x;
  case 10: return l;
  case 11: return r;
  }

  return 0;  //12-15: signature
}

auto SFCGamepad::data2() -> uint5 {
  return 0;
}

auto SFCGamepad::write(uint3 data) -> void {
  if(latched == data.bit(0)) return;
  latched = data.bit(0);
  counter = 0;

  if(latched == 0) {
    b      = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, B);
    y      = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, Y);
    select = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, Select);
    start  = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, Start);
    up     = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, Up);
    down   = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, Down);
    left   = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, Left);
    right  = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, Right);
    a      = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, A);
    x      = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, X);
    l      = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, L);
    r      = platform->inputPoll(ID::Port::Expansion, ID::Device::SFCGamepad, R);
  }
}
