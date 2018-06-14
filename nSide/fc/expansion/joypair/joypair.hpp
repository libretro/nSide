struct JoyPair : Expansion {
  enum : uint {
    Up, Down, Left, Right, B, A, Select, Start,
  };

  JoyPair();

  auto data1() -> bool;
  auto data2() -> uint5;
  auto write(uint3 data) -> void;

private:
  bool latched;
  uint counter1;
  uint counter2;

  struct Gamepad {
    boolean a, b, select, start;
    boolean up, down, left, right;
  } gamepads[2];
};
