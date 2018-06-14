#include <a2600/a2600.hpp>

namespace Atari2600 {

System system;
Scheduler scheduler;
Random random;
Cheat cheat;
#include "video.cpp"
#include "serialization.cpp"

auto System::run() -> void {
  if(scheduler.enter() == Scheduler::Event::Frame) tia.refresh();
}

auto System::runToSave() -> void {
  scheduler.synchronize(cpu);
  scheduler.synchronize(pia);
  scheduler.synchronize(tia);
  for(auto peripheral : pia.peripherals) scheduler.synchronize(*peripheral);
}

auto System::load(Emulator::Interface* interface) -> bool {
  information = {};

  if(auto fp = platform->open(ID::System, "manifest.bml", File::Read, File::Required)) {
    information.manifest = fp->reads();
  } else return false;

  auto document = BML::unserialize(information.manifest);
  auto system = document["system"];

  if(!cartridge.load()) return false;

  if(cartridge.region() == "NTSC") {
    information.region = Region::NTSC;
    information.frequency = Emulator::Constants::Colorburst::NTSC;
  }
  if(cartridge.region() == "PAL") {
    information.region = Region::PAL;
    information.frequency = Emulator::Constants::Colorburst::PAL * 4.0 / 5.0;
  }
  if(cartridge.region() == "SECAM") {
    information.region = Region::SECAM;
    information.frequency = Emulator::Constants::Colorburst::PAL * 4.0 / 5.0;
  }

  if(!cpu.load(system)) return false;
  if(!pia.load(system)) return false;
  if(!tia.load(system)) return false;

  serializeInit();
  this->interface = interface;
  return information.loaded = true;
}

auto System::save() -> void {
  if(!loaded()) return;

  cartridge.save();
}

auto System::unload() -> void {
  if(!loaded()) return;

  pia.peripherals.reset();
  controllerPort1.unload();
  controllerPort2.unload();

  cartridge.unload();
  information.loaded = false;
}

auto System::power() -> void {
  Emulator::video.reset();
  Emulator::video.setInterface(interface);
  configureVideoPalette();
  configureVideoEffects();

  Emulator::audio.reset();
  Emulator::audio.setInterface(interface);

  random.entropy(Random::Entropy::Low);

  scheduler.reset();
  cpu.power();
  pia.power();
  tia.power();
  scheduler.primary(cpu);

  controllerPort1.power(ID::Port::Controller1);
  controllerPort2.power(ID::Port::Controller2);

  controllerPort1.connect(settings.controllerPort1);
  controllerPort2.connect(settings.controllerPort2);
}

}
