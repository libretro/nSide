struct MouseE : Expansion {
  enum : uint {
    X, Y, Left, Right,
  };

  MouseE();

  auto data1() -> bool;
  auto data2() -> uint5;
  auto write(uint3 data) -> void;

private:
  bool latched;
  uint counter;

  uint speed;  //0 = slow, 1 = normal, 2 = fast
  int x;       //x-coordinate
  int y;       //y-coordinate
  bool dx;     //x-direction
  bool dy;     //y-direction
  bool l;      //left button
  bool r;      //right button
};
