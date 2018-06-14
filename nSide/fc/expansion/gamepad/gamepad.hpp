struct GamepadE : Expansion {
  enum : uint {
    Up, Down, Left, Right, B, A, Select, Start,
  };

  GamepadE();

  auto data1() -> bool;
  auto data2() -> uint5;
  auto write(uint3 data) -> void;

private:
  bool latched;
  uint counter;

  boolean a, b, select, start;
  boolean up, down, left, right;
};
