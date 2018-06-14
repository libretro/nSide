//MOS 6532 RIOT

struct PIA : Thread {
  PIA();
  ~PIA();

  alwaysinline auto step(uint clocks) -> void;

  static auto Enter() -> void;
  auto main() -> void;
  auto load(Markup::Node) -> bool;
  auto power() -> void;

  auto updateIO() -> void;
  auto runTimer() -> void;

  //memory.cpp
  auto readRAM(uint7 addr, uint8 data) -> uint8;
  auto writeRAM(uint7 addr, uint8 data) -> void;

  //io.cpp
  auto readIO(uint7 addr, uint8 data) -> uint8;
  auto writeIO(uint7 addr, uint8 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  uint8 ram[128];
  vector<Thread*> peripherals;

private:
  struct TVType { enum : bool { Monochrome, Color };};
  struct Difficulty { enum : bool { B, A };};

  struct IO {
    bool tvtype;
    bool difficulty0;
    bool difficulty1;

    //$0280 SWCHA
    uint8 swcha;

    //$0281 SWACNT
    uint8 swacnt;

    //$0282 SWCHB
    uint8 swchb;

    //$0283 SWBCNT
    uint8 swbcnt;

    union {
      uint value;
      NaturalBitField<uint, 0, 9> prescaler;
      NaturalBitField<uint,10,17> base;
    } timer;
    uint10 timerDecrement;
    bool timerUnderflowINSTAT;
    bool timerUnderflowTIM_T;

    bool timerIRQEnable;  //useless with MOS 6507
    bool pa7IRQEnable;  //useless with MOS 6507
    bool pa7EdgeDetect;
  } io;

  struct Input {
    bool tvtype;
    bool difficulty0;
    bool difficulty1;
  } input;
};

extern PIA pia;
