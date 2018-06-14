#include <fc/fc.hpp>

namespace Famicom {

#define cpu (side ? cpuS : cpuM)
#define ppu (side ? ppuS : ppuM)

ControllerPort controllerPortM1;
ControllerPort controllerPortM2;
ControllerPort controllerPortS1;
ControllerPort controllerPortS2;
#include "gamepad/gamepad.cpp"
#include "gamepad-mic/gamepad-mic.cpp"
#include "four-score/four-score.cpp"
#include "zapper/zapper.cpp"
#include "power-pad/power-pad.cpp"
#include "vaus/vaus.cpp"
#include "snes-gamepad/snes-gamepad.cpp"
#include "mouse/mouse.cpp"

Controller::Controller(bool side, uint port) : side(side), port(port) {
  if(!handle()) create(Controller::Enter, 1);
}

Controller::~Controller() {
  scheduler.remove(*this);
}

auto Controller::Enter() -> void {
  while(true) {
    scheduler.synchronize();
    if(!Model::VSSystem() || vssystem.gameCount == 2) {
      if(controllerPortM1.device->active()) controllerPortM1.device->main();
      if(controllerPortM2.device->active()) controllerPortM2.device->main();
    }
    if(Model::VSSystem()) {
      if(controllerPortS1.device->active()) controllerPortS1.device->main();
      if(controllerPortS2.device->active()) controllerPortS2.device->main();
    }
  }
}

auto Controller::main() -> void {
  step(1);
  synchronize(cpu);
}

//

auto ControllerPort::connect(uint deviceID) -> void {
  if(!system.loaded()) return;
  delete device;

  switch(deviceID) { default:
  case ID::Device::None: device = new Controller(side, port); break;
  case ID::Device::Gamepad: device = new Gamepad(side, port); break;
  case ID::Device::GamepadMic: device = new GamepadMic(side, port); break;
  case ID::Device::FourScore: device = new FourScore(side, port); break;
  case ID::Device::Zapper: device = new Zapper(side, port); break;
  case ID::Device::PowerPad: device = new PowerPad(side, port); break;
  case ID::Device::Vaus: device = new Vaus(side, port); break;
  case ID::Device::SNESGamepad: device = new SNESGamepad(side, port); break;
  case ID::Device::Mouse: device = new Mouse(side, port); break;
  }

  //The Four Score is a single device that consumes both controller ports.
  //Do now allow only half of a Four Score to be connected.
  if(!Model::VSSystem() && (port == ID::Port::Controller1 || port == ID::Port::Controller2)) {
    auto& oppositePort = (port == ID::Port::Controller1
    ? controllerPortM2
    : controllerPortM1
    );
    auto& oppositeDeviceID = (port == ID::Port::Controller1
    ? settings.controllerPort2
    : settings.controllerPort1
    );
    if(deviceID == ID::Device::FourScore && oppositeDeviceID != ID::Device::FourScore) {
      oppositePort.connect(oppositeDeviceID = ID::Device::FourScore);
      return;
    } else if(deviceID != ID::Device::FourScore && oppositeDeviceID == ID::Device::FourScore) {
      oppositePort.connect(oppositeDeviceID = ID::Device::None);
      return;
    }
  }

  cpu.peripherals.reset();
  if(side == 0) {
    if(auto device = controllerPortM1.device) cpuM.peripherals.append(device);
    if(auto device = controllerPortM2.device) cpuM.peripherals.append(device);
  } else if(side == 1) {
    if(auto device = controllerPortS1.device) cpuS.peripherals.append(device);
    if(auto device = controllerPortS2.device) cpuS.peripherals.append(device);
  }
  if(!Model::VSSystem()) if(auto device = expansionPort.device) cpuM.peripherals.append(device);
}

auto ControllerPort::power(bool side, uint port) -> void {
  this->side = side;
  this->port = port;
}

auto ControllerPort::unload() -> void {
  delete device;
  device = nullptr;
}

auto ControllerPort::serialize(serializer& s) -> void {
}

#undef cpu
#undef ppu

}
