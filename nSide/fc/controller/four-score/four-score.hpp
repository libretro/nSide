struct FourScore : Controller {
  enum : uint {
    Up, Down, Left, Right, B, A, Select, Start,
  };

  FourScore(bool side, uint port);

  auto data() -> uint3;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter;

  struct Gamepad {
    boolean a, b, select, start;
    boolean up, down, left, right;
  } gamepads[2];
};
