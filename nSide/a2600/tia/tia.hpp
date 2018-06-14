//Television Interface Adaptor

struct TIA : Thread {
  shared_pointer<Emulator::Stream> stream;

  TIA();
  ~TIA();

  alwaysinline auto step(uint clocks) -> void;

  static auto Enter() -> void;
  auto main() -> void;
  auto load(Markup::Node) -> bool;
  auto power() -> void;

  //io.cpp
  auto readIO(uint6 addr, uint8 data) -> uint8;
  auto writeIO(uint6 addr, uint8 data) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

private:
  struct IO {
    uint8 hcounter;
    uint vcounter;  //not actually in TIA; needed for rendering

    //$0000 VSYNC
    bool vsync;

    //$0001 VBLANK
    bool vblank;

    //$0006 COLUP0
    uint7 playerMissile0Color;

    //$0007 COLUP1
    uint7 playerMissile1Color;

    //$0008 COLUPF
    uint7 playfieldBallColor;

    //$0009 COLUBK
    uint7 backgroundColor;

    //$000a CTRLPF
    bool playfieldBallPriority;
  } io;

  struct Player {
    uint3 numberSize;  //also affects missiles
    bool reflect;
    uint8 graphic;
    uint8 position;
    int3 motion;
  } player[2];

  struct Missile {
    bool enable;
    uint2 size;
    uint8 position;
    int3 motion;
    bool reset;
  } missile[2];

  struct Ball {
    bool enable;
    uint2 size;
    uint8 position;
    int3 motion;
  } ball;

  struct Playfield {
    bool reflect;
    bool score;

    uint4 graphic0;
    uint8 graphic1;
    uint8 graphic2;
  } playfield;

  struct HMove {
    bool enable;
    uint2 counter;
    uint3 delay;

    bool latchP0;
    bool latchP1;
    bool latchM0;
    bool latchM1;
    bool latchBL;

    uint4 counterP0;
    uint4 counterP1;
    uint4 counterM0;
    uint4 counterM1;
    uint4 counterBL;
  } hmove;

  struct Collision {
    bool m0p1;
    bool m0p0;
    bool m1p0;
    bool m1p1;
    bool p0pf;
    bool p0bl;
    bool p1pf;
    bool p1bl;
    bool m0pf;
    bool m0bl;
    bool m1pf;
    bool m1bl;
    bool blpf;
    bool p0p1;
    bool m0m1;
  } collision;

  struct Audio {
    struct Channel {
      uint4 control;
      uint5 frequency;
      uint4 volume;

      uint5 phase;
      bool state1;
      uint2 state3;
      uint4 shift4;
      uint5 shift5;
      uint9 shift9;
      bool sample;
    } channel[2];
  } audio;

  alwaysinline auto hblank() -> bool { return io.hcounter < 68; }

  auto refresh() -> void;

  //render.cpp
  auto frame() -> void;
  auto scanline() -> void;
  auto runVideo() -> void;

  //audio.cpp
  auto runAudio() -> void;
  alwaysinline auto shift(uint4& shift) -> bool;
  alwaysinline auto shift(uint5& shift) -> bool;
  alwaysinline auto shift(uint9& shift) -> bool;

  friend class System;

  uint32* output = nullptr;
};

extern TIA tia;
