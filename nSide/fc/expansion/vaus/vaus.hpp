struct VausE : Expansion {
  enum : uint {
    Control, Fire,
  };

  VausE();

  auto main() -> void;
  auto data1() -> bool;
  auto data2() -> uint5;
  auto write(uint3 data) -> void;

private:
  bool latched;
  uint counter;

  int x;
  uint8 control;

  uint prev;
};
