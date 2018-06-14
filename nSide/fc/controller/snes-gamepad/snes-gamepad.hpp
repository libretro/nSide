struct SNESGamepad : Controller {
  enum : uint {
    Up, Down, Left, Right, B, A, Y, X, L, R, Select, Start,
  };

  SNESGamepad(bool side, uint port);

  auto data() -> uint3;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter;

  boolean b, y, select, start;
  boolean up, down, left, right;
  boolean a, x, l, r;
};
