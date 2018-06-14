struct BeamGun : Expansion {
  shared_pointer<Emulator::Sprite> sprite;

  enum : uint {
    X, Y, Trigger,
  };

  BeamGun();
  ~BeamGun();

  auto main() -> void;
  auto data1() -> bool;
  auto data2() -> uint5;
  auto readLight() -> bool;
  auto write(uint3 data) -> void;

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
