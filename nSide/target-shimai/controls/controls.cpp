#include "../shimai.hpp"
#include "fc/interface/interface.hpp"
unique_pointer<Controls> controls;

Controls::Controls() {
  controls = this;
}

auto Controls::reset() -> void {
  for(auto& emulator : program->emulators) {
    if(emulator->information.name == "Famicom") {
      inputManager->bind(emulator);
      break;
    }
  }

  up     = false;
  down   = false;
  left   = false;
  right  = false;
  b      = false;
  a      = false;
  select = false;
  start  = false;
  previousUp     = false;
  previousDown   = false;
  previousLeft   = false;
  previousRight  = false;
  previousB      = false;
  previousA      = false;
  previousSelect = false;
  previousStart  = false;

  onPressUp     = {};
  onPressDown   = {};
  onPressLeft   = {};
  onPressRight  = {};
  onPressB      = {};
  onPressA      = {};
  onPressSelect = {};
  onPressStart  = {};
}

auto Controls::run() -> void {
  poll();

  if(onPressUp     && up     && !previousUp    ) onPressUp    ();
  if(onPressDown   && down   && !previousDown  ) onPressDown  ();
  if(onPressLeft   && left   && !previousLeft  ) onPressLeft  ();
  if(onPressRight  && right  && !previousRight ) onPressRight ();
  if(onPressB      && b      && !previousB     ) onPressB     ();
  if(onPressA      && a      && !previousA     ) onPressA     ();
  if(onPressSelect && select && !previousSelect) onPressSelect();
  if(onPressStart  && start  && !previousStart ) onPressStart ();
}

auto Controls::poll() -> void {
  uint port1 = Famicom::ID::Port::Controller1;
  uint gamepad = Famicom::ID::Device::Gamepad;
  enum : uint {Up, Down, Left, Right, B, A, Select, Start};

  previousUp     = up;
  previousDown   = down;
  previousLeft   = left;
  previousRight  = right;
  previousB      = b;
  previousA      = a;
  previousSelect = select;
  previousStart  = start;

  up     = program->inputPoll(port1, gamepad, Up);
  down   = program->inputPoll(port1, gamepad, Down);
  left   = program->inputPoll(port1, gamepad, Left);
  right  = program->inputPoll(port1, gamepad, Right);
  b      = program->inputPoll(port1, gamepad, B);
  a      = program->inputPoll(port1, gamepad, A);
  select = program->inputPoll(port1, gamepad, Select);
  start  = program->inputPoll(port1, gamepad, Start);
}
