Joystick::Joystick(uint port) : Controller(port) {
}

auto Joystick::pot0() -> bool {
  return 1;
}

auto Joystick::pot1() -> bool {
  return 1;
}

auto Joystick::fire() -> bool {
  return !platform->inputPoll(port, ID::Device::Joystick, Fire);
}

auto Joystick::direction() -> uint4 {
  bool up    = platform->inputPoll(port, ID::Device::Joystick, Up);
  bool down  = platform->inputPoll(port, ID::Device::Joystick, Down);
  bool left  = platform->inputPoll(port, ID::Device::Joystick, Left);
  bool right = platform->inputPoll(port, ID::Device::Joystick, Right);
  return !(up && !down) << 0 | !(down && !up) << 1 | !(left && !right) << 2 | !(right && !left) << 3;
}
