#include <md/md.hpp>

namespace MegaDrive {

ControllerPort controllerPort1;
ControllerPort controllerPort2;
ControllerPort extensionPort;
#include "control-pad/control-pad.cpp"
#include "fighting-pad/fighting-pad.cpp"
#include "mouse/mouse.cpp"
#include "sega-tap/sega-tap.cpp"
#include "ea-4-way-play/ea-4-way-play.cpp"

Controller::Controller(uint port) : port(port) {
  if(!handle()) create(Controller::Enter, 1);
}

Controller::~Controller() {
  scheduler.remove(*this);
}

auto Controller::Enter() -> void {
  while(true) {
    scheduler.synchronize();
    if(controllerPort1.device->active()) controllerPort1.device->main();
    if(controllerPort2.device->active()) controllerPort2.device->main();
    if(extensionPort.device->active()) extensionPort.device->main();
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
  case ID::Device::None: device = new Controller(port); break;
  case ID::Device::ControlPad: device = new ControlPad(port); break;
  case ID::Device::FightingPad: device = new FightingPad(port); break;
  case ID::Device::Mouse: device = new Mouse(port); break;
  case ID::Device::SegaTap: device = new SegaTap(port); break;
  case ID::Device::EA4WayPlay: device = new EA4WayPlay(port); break;
  }

  //The EA 4 Way Play is a single device that consumes both controller ports.
  //Do now allow only half of an EA 4 Way Play to be connected.
  if(port == ID::Port::Controller1 || port == ID::Port::Controller2) {
    auto& oppositePort = (port == ID::Port::Controller1
    ? controllerPort2
    : controllerPort1
    );
    auto& oppositeDeviceID = (port == ID::Port::Controller1
    ? settings.controllerPort2
    : settings.controllerPort1
    );
    if(deviceID == ID::Device::EA4WayPlay && oppositeDeviceID != ID::Device::EA4WayPlay) {
      oppositePort.connect(oppositeDeviceID = ID::Device::EA4WayPlay);
      return;
    } else if(deviceID != ID::Device::EA4WayPlay && oppositeDeviceID == ID::Device::EA4WayPlay) {
      oppositePort.connect(oppositeDeviceID = ID::Device::None);
      return;
    }
  }

  cpu.peripherals.reset();
  if(auto device = controllerPort1.device) cpu.peripherals.append(device);
  if(auto device = controllerPort2.device) cpu.peripherals.append(device);
  if(auto device = extensionPort.device) cpu.peripherals.append(device);
}

auto ControllerPort::readData() -> uint8 {
  return device->readData() | latch << 7;
}

auto ControllerPort::writeData(uint8 data) -> void {
  device->writeData(data);
  latch = data.bit(7);
}

auto ControllerPort::readControl() -> uint8 {
  return control;
}

auto ControllerPort::writeControl(uint8 data) -> void {
  control = data;
}

auto ControllerPort::power(uint port) -> void {
  this->port = port;
  control = 0x00;
}

auto ControllerPort::unload() -> void {
  delete device;
  device = nullptr;
}

auto ControllerPort::serialize(serializer& s) -> void {
  s.integer(control);
}

}
