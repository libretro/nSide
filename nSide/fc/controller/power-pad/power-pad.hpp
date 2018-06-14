struct PowerPad : Controller {
  PowerPad(bool side, uint port);

  auto data() -> uint3;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter;

  bool b1, b2,  b3,  b4;
  bool b5, b6,  b7,  b8;
  bool b9, b10, b11, b12;
};
