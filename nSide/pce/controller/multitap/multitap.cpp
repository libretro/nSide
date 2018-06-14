Multitap::Multitap() {
}

auto Multitap::readData() -> uint4 {
  if(clr || id >= 5) return 0;

  uint4 data;

  if(sel) {
    bool up    = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + Up);
    bool right = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + Right);
    bool down  = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + Down);
    bool left  = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + Left);
    data.bit(0) = !(up & !down);
    data.bit(1) = !(right & !left);
    data.bit(2) = !(down & !up);
    data.bit(3) = !(left & !right);
  } else {
    bool one    = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + One);
    bool two    = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + Two);
    bool select = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + Select);
    bool run    = platform->inputPoll(ID::Port::Controller, ID::Device::Multitap, id * 8 + Run);
    data.bit(0) = !one;
    data.bit(1) = !two;
    data.bit(2) = !select;
    data.bit(3) = !run;
  }

  return data;
}

auto Multitap::writeData(uint2 data) -> void {
  if(!sel && data.bit(0)) id++;
  sel = data.bit(0);
  clr = data.bit(1);
  if(clr) id = 0;
}
