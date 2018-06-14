struct Joystick : Controller {
  enum : uint {
    Up, Down, Left, Right, Fire,
  };

  Joystick(uint port);

  auto pot0() -> bool;
  auto pot1() -> bool;
  auto fire() -> bool;
  auto direction() -> uint4;
};
