//NES/AV Famicom controller port pinout:
//  ____
// |    \
// |(7)  \
// |(2)(1)|
// |(3)(5)|
// |(4)(6)|
// |______|
//
// pin  name   port1           port2
//  1:  +5v
//  2:  clock  $4016 read      $4017 read
//  3:  latch  $4016.d0 write  $4016.d0 write
//  4:  data0  $4016.d0 read   $4017.d0 read
//  5:  data3  $4016.d3 read   $4017.d3 read
//  6:  data4  $4016.d4 read   $4017.d4 read
//  7:  gnd

struct Controller : Thread {
  Controller(bool side, uint port);
  virtual ~Controller();
  static auto Enter() -> void;

  virtual auto main() -> void;
  virtual auto data() -> uint3 { return 0; }
  virtual auto mic() -> bool { return 0; }
  virtual auto latch(bool data) -> void {}

  const bool side;
  const uint port;
};

struct ControllerPort {
  auto connect(uint deviceID) -> void;

  auto power(bool side, uint port) -> void;
  auto unload() -> void;
  auto serialize(serializer&) -> void;

  bool side;
  uint port;
  Controller* device = nullptr;
};

extern ControllerPort controllerPortM1;
extern ControllerPort controllerPortM2;
extern ControllerPort controllerPortS1;
extern ControllerPort controllerPortS2;

#include "gamepad/gamepad.hpp"
#include "gamepad-mic/gamepad-mic.hpp"
#include "four-score/four-score.hpp"
#include "zapper/zapper.hpp"
#include "power-pad/power-pad.hpp"
#include "vaus/vaus.hpp"
#include "snes-gamepad/snes-gamepad.hpp"
#include "mouse/mouse.hpp"
