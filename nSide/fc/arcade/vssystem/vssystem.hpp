struct VSSystem : Thread {
  enum : uint {
    Button1, Button2, Button3, Button4, ServiceButton, Coin1, Coin2,
  };

  struct GameCount { enum : uint {
    Uni = 1,
    Dual = 2,
  };};

  VSSystem();

  static auto Enter() -> void;
  auto main() -> void;

  auto load() -> bool;
  auto power(bool reset) -> void;

  auto setDip(bool side, uint8 dip) -> void;
  auto read(bool side, uint16 addr, uint8 data) -> uint8;
  auto write(bool side, uint16 addr, uint8 data) -> void;

  auto serialize(serializer& s) -> void;

  bool forceSubRAM;
  uint gameCount;
  bool swapControllersM;
  bool swapControllersS;

private:
  uint8 ram[0x800];
  bool ramSide; // 0: main, 1: sub

  uint8 dipM;
  uint8 dipS;
  uint watchdog;

  //control.cpp
  bool controlLatchedM;
  uint controlCounterM1;
  uint controlCounterM2;
  bool buttonsM[4];
  bool controlLatchedS;
  uint controlCounterS1;
  uint controlCounterS2;
  bool buttonsS[4];

  auto resetButtons() -> void;
  auto poll(bool side, uint input) -> int16;
  auto data1(bool side) -> bool;
  auto data2(bool side) -> bool;
  auto latch(bool side, bool data) -> void;

  friend class Cartridge;
};

extern VSSystem vssystem;
