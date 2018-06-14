struct Gamepad : Controller {
  enum : uint {
    Up, Down, Left, Right, B, A, Select, Start,
  };

  Gamepad(bool side, uint port);

  auto data() -> uint3;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter;

  boolean a, b, select, start;
  boolean up, down, left, right;

  friend class PlayChoice10;
};
