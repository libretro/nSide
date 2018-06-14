#include <fc/fc.hpp>

namespace Famicom {

#define cpu cpuM  //VS. System does not use expansion port devices
#define ppu ppuM

ExpansionPort expansionPort;
#include "gamepad/gamepad.cpp"
#include "joypair/joypair.cpp"
#include "four-players/four-players.cpp"
#include "beam-gun/beam-gun.cpp"
#include "family-keyboard/family-keyboard.cpp"
#include "family-trainer/family-trainer.cpp"
#include "vaus/vaus.cpp"
#include "sfc-gamepad/sfc-gamepad.cpp"
#include "mouse/mouse.cpp"

Expansion::Expansion() {
  if(!handle()) create(Expansion::Enter, 1);
}

Expansion::~Expansion() {
  scheduler.remove(*this);
}

auto Expansion::Enter() -> void {
  while(true) {
    scheduler.synchronize();
    if(!Model::VSSystem()) expansionPort.device->main();
  }
}

auto Expansion::main() -> void {
  step(1);
  synchronize(cpu);
}

//

auto ExpansionPort::connect(uint deviceID) -> void {
  if(!system.loaded()) return;
  delete device;

  switch(deviceID) { default:
    case ID::Device::None: device = new Expansion; break;
    case ID::Device::GamepadE: device = new GamepadE; break;
    case ID::Device::JoyPair: device = new JoyPair; break;
    case ID::Device::FourPlayers: device = new FourPlayers; break;
    case ID::Device::BeamGun: device = new BeamGun; break;
    case ID::Device::FamilyKeyboard: device = new FamilyKeyboard; break;
    case ID::Device::FamilyTrainer: device = new FamilyTrainer; break;
    case ID::Device::VausE: device = new VausE; break;
    case ID::Device::SFCGamepad: device = new SFCGamepad; break;
    case ID::Device::MouseE: device = new MouseE; break;
  }

  cpu.peripherals.reset();
  if(auto device = controllerPortM1.device) cpu.peripherals.append(device);
  if(auto device = controllerPortM2.device) cpu.peripherals.append(device);
  if(auto device = expansionPort.device) cpu.peripherals.append(device);
}

auto ExpansionPort::power() -> void {
}

auto ExpansionPort::unload() -> void {
  delete device;
  device = nullptr;
}

auto ExpansionPort::serialize(serializer& s) -> void {
}

#undef cpu
#undef ppu

}
