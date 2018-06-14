struct Zapper : Controller {
  shared_pointer<Emulator::Sprite> sprite;

  enum : uint {
    X, Y, Trigger,
  };

  Zapper(bool side, uint port);
  ~Zapper();

  auto main() -> void;
  auto data() -> uint3;
  auto readLight() -> bool;
  auto latch(bool data) -> void;

private:
  bool latched;
  uint counter; // VS. System

  int x;
  int y;

  bool light;
  bool trigger;
  uint lighttime;
  uint triggertime;
  bool offscreen;

  bool triggerlock;

  uint prev;
};
