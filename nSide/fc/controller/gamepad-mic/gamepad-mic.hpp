struct GamepadMic : Controller {
  enum : uint {
    Up, Down, Left, Right, B, A, Mic,
  };

  GamepadMic(bool side, uint port);

  auto data() -> uint3;
  auto mic() -> bool;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter;

  boolean a, b;
  boolean up, down, left, right;
};
