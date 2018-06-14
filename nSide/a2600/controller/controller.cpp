#include <a2600/a2600.hpp>

namespace Atari2600 {

ControllerPort controllerPort1;
ControllerPort controllerPort2;
#include "joystick/joystick.cpp"

Controller::Controller(uint port) : port(port) {
  if(!handle()) create(Controller::Enter, 1);
}

Controller::~Controller() {
}

auto Controller::Enter() -> void {
  while(true) {
    scheduler.synchronize();
    if(controllerPort1.device->active()) controllerPort1.device->main();
    if(controllerPort2.device->active()) controllerPort2.device->main();
  }
}

auto Controller::main() -> void {
  step(1);
  synchronize(pia);
}

//

auto ControllerPort::connect(uint deviceID) -> void {
  if(!system.loaded()) return;
  delete device;

  switch(deviceID) { default:
  case ID::Device::None: device = new Controller(port); break;
  case ID::Device::Joystick: device = new Joystick(port); break;
  }

  pia.peripherals.reset();
  if(auto device = controllerPort1.device) pia.peripherals.append(device);
  if(auto device = controllerPort2.device) pia.peripherals.append(device);
}

auto ControllerPort::power(uint port) -> void {
  this->port = port;
}

auto ControllerPort::unload() -> void {
  delete device;
  device = nullptr;
}

auto ControllerPort::serialize(serializer& s) -> void {
}

}
