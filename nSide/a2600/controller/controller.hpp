//Atari 2600 controller port pinout:
//  ___________________
// /(1) (2) (3) (4) (5)\
//  \ (6) (7) (8) (9) /
//   \---------------/
//
// pin  name     port1      port2
//  1:  up       $0280.d4   $0280.d0
//  2:  down     $0280.d5   $0280.d1
//  3:  left     $0280.d6   $0280.d2
//  4:  right    $0280.d7   $0280.d3
//  5:  inpt0/2  $0038.d7   $003a.d7
//  6:  inpt4/5  $003c.d7   $003d.d7
//  7:  +5v
//  8:  gnd
//  9:  inpt1/3  $0039.d7   $003b.d7

struct Controller : Thread {
  Controller(uint port);
  virtual ~Controller();
  static auto Enter() -> void;

  virtual auto main() -> void;
  virtual auto pot0() -> bool { return 1; }
  virtual auto pot1() -> bool { return 1; }
  virtual auto fire() -> bool { return 1; }
  virtual auto direction() -> uint4 { return 0xf; }

  const uint port;
};

struct ControllerPort {
  auto connect(uint deviceID) -> void;

  auto power(uint port) -> void;
  auto unload() -> void;
  auto serialize(serializer&) -> void;

  uint port;
  Controller* device = nullptr;
};

extern ControllerPort controllerPort1;
extern ControllerPort controllerPort2;

#include "joystick/joystick.hpp"
