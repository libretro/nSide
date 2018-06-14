struct Vaus : Controller {
  enum : uint {
    Control, Fire,
  };

  Vaus(bool side, uint port);

  auto main() -> void;
  auto data() -> uint3;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter;

  int x;
  uint8 control;

  uint prev;
};
