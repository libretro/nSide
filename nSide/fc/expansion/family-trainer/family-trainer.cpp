FamilyTrainer::FamilyTrainer() {
}

auto FamilyTrainer::data1() -> bool {
  return 0;
}

auto FamilyTrainer::data2() -> uint5 {
  uint5 result = 0x00;
  #define port ID::Port::Expansion
  #define device ID::Device::FamilyTrainer
  if(!mask.bit(2)) {
    result.bit(4) |= !platform->inputPoll(port, device,  0);
    result.bit(3) |= !platform->inputPoll(port, device,  1);
    result.bit(2) |= !platform->inputPoll(port, device,  2);
    result.bit(1) |= !platform->inputPoll(port, device,  3);
  }
  if(!mask.bit(1)) {
    result.bit(4) |= !platform->inputPoll(port, device,  4);
    result.bit(3) |= !platform->inputPoll(port, device,  5);
    result.bit(2) |= !platform->inputPoll(port, device,  6);
    result.bit(1) |= !platform->inputPoll(port, device,  7);
  }
  if(!mask.bit(0)) {
    result.bit(4) |= !platform->inputPoll(port, device,  8);
    result.bit(3) |= !platform->inputPoll(port, device,  9);
    result.bit(2) |= !platform->inputPoll(port, device, 10);
    result.bit(1) |= !platform->inputPoll(port, device, 11);
  }
  #undef port
  #undef device
  return result;
}

auto FamilyTrainer::write(uint3 data) -> void {
  mask = data;
}
