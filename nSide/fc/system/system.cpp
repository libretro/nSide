#include <fc/fc.hpp>

namespace Famicom {

System system;
Scheduler scheduler;
Random random;
Cheat cheat;
#include "video.cpp"
#include "serialization.cpp"

auto System::run() -> void {
  if(scheduler.enter() == Scheduler::Event::Frame) {
    if(Famicom::Model::FamicomBox()) famicombox.pollInputs();
    if(!Famicom::Model::VSSystem() || vssystem.gameCount == 2) ppuM.refresh();
    if(Famicom::Model::VSSystem()) ppuS.refresh();
    if(Famicom::Model::PlayChoice10()) playchoice10.videoCircuit.refresh();
    Emulator::video.refreshFinal();
  }
}

auto System::runToSave() -> void {
  if(!Famicom::Model::VSSystem() || vssystem.gameCount == 2) {
    scheduler.synchronize(cpuM);
    scheduler.synchronize(apuM);
    scheduler.synchronize(ppuM);
  }

  if(Famicom::Model::VSSystem()) {
    scheduler.synchronize(cpuS);
    scheduler.synchronize(apuS);
    scheduler.synchronize(ppuS);
  }

  if(!Famicom::Model::VSSystem() || vssystem.gameCount == 2) {
    scheduler.synchronize(cartridgeSlot[busM.slot]);
    for(auto coprocessor : cpuM.coprocessors) scheduler.synchronize(*coprocessor);
    for(auto peripheral : cpuM.peripherals) scheduler.synchronize(*peripheral);
  }

  if(Famicom::Model::VSSystem()) {
    scheduler.synchronize(cartridgeSlot[busS.slot]);
    for(auto coprocessor : cpuS.coprocessors) scheduler.synchronize(*coprocessor);
    for(auto peripheral : cpuS.peripherals) scheduler.synchronize(*peripheral);
  }
}

auto System::load(Emulator::Interface* interface, Model model) -> bool {
  information = {};
  information.model = model;

  if(auto fp = platform->open(ID::System, "manifest.bml", File::Read, File::Required)) {
    information.manifest = fp->reads();
  } else return false;

  auto document = BML::unserialize(information.manifest);
  auto system = document["system"];

  busM.slot = 0;
  busM.reset();
  busS.slot = 1;
  busS.reset();
  cartridgeSlot.append(Cartridge(0));
  if(Famicom::Model::VSSystem()) cartridgeSlot.append(Cartridge(1));
  if(!cartridgeSlot[0].load()) return false;

  if(Famicom::Model::PlayChoice10()) for(uint slot : range(1, 10)) {
    auto cartridge = Cartridge(slot);
    if(!cartridge.load()) break;
    cartridgeSlot.append(cartridge);
  }
  if(Famicom::Model::FamicomBox()) for(uint slot : range(1, 15)) {
    auto cartridge = Cartridge(slot);
    if(!cartridge.load()) break;
    cartridgeSlot.append(cartridge);
  }

  if(Famicom::Model::Famicom()) {
    if(cartridgeSlot[0].region() == "NTSC-J") {
      information.region = Region::NTSCJ;
      information.frequency = Emulator::Constants::Colorburst::NTSC * 6.0;
    }
    if(cartridgeSlot[0].region() == "NTSC-U") {
      information.region = Region::NTSCU;
      information.frequency = Emulator::Constants::Colorburst::NTSC * 6.0;
    }
    if(cartridgeSlot[0].region() == "PAL") {
      information.region = Region::PAL;
      information.frequency = Emulator::Constants::Colorburst::PAL * 6.0;
    }
    if(cartridgeSlot[0].region() == "Dendy") {
      information.region = Region::Dendy;
      information.frequency = Emulator::Constants::Colorburst::PAL * 6.0;
    }
  } else {
    information.region = Famicom::Model::PlayChoice10() ? Region::NTSCU : Region::NTSCJ;
    information.frequency = Emulator::Constants::Colorburst::NTSC * 6.0;
  }

  if(!cpuM.load(system)) return false;
  if(!apuM.load(system)) return false;
  if(!ppuM.load(system)) return false;

  if(Famicom::Model::VSSystem()) {
    if(!cpuS.load(system)) return false;
    if(!apuS.load(system)) return false;
    if(!ppuS.load(system)) return false;
  }

  switch(model) {

  case Model::VSSystem: {
    vssystem.load();
    break;
  }

  case Model::PlayChoice10: {
    if(!playchoice10.load(system)) return false;
    break;
  }

  case Model::FamicomBox: {
    if(!famicombox.load(system)) return false;
    break;
  }

  }

  serializeInit();
  this->interface = interface;
  return information.loaded = true;
}

auto System::save() -> void {
  if(!loaded()) return;

  if(!Famicom::Model::VSSystem()) {
    for(auto& cartridge : cartridgeSlot) cartridge.save();
  } else {
    cartridgeSlot[2 - vssystem.gameCount].save();
  }
}

auto System::unload() -> void {
  if(!loaded()) return;

  cpuM.peripherals.reset();
  cpuS.peripherals.reset();
  controllerPortM1.unload();
  controllerPortM2.unload();
  expansionPort.unload();
  controllerPortS1.unload();
  controllerPortS2.unload();

  if(!Famicom::Model::VSSystem()) {
    for(auto& cartridge : cartridgeSlot) cartridge.unload();
  } else {
    cartridgeSlot[2 - vssystem.gameCount].unload();
  }

  if(Famicom::Model::FamicomBox()) famicombox.unload();

  cartridgeSlot.reset();
  information.loaded = false;
}

auto System::power(bool reset) -> void {
  Emulator::video.reset();
  Emulator::video.setInterface(interface);
  //Emulator::video.resize() is called in configureVideoEffects()
  configureVideoPalette();
  configureVideoEffects();

  Emulator::audio.reset();
  Emulator::audio.setInterface(interface);

  random.entropy(Random::Entropy::Low);

  scheduler.reset();

  if(!Famicom::Model::VSSystem() || vssystem.gameCount == 2) {
    cartridgeSlot[busM.slot].power(reset);
    cpuM.power(reset);
    apuM.power(reset);
    ppuM.power(reset);
  }

  if(Famicom::Model::VSSystem()) {
    cartridgeSlot[busS.slot].power(reset);
    cpuS.power(reset);
    apuS.power(reset);
    ppuS.power(reset);
  }

  switch(model()) {
  case Model::VSSystem:     vssystem.power(reset); break;
  case Model::PlayChoice10: playchoice10.power(reset); break;
  case Model::FamicomBox:   famicombox.power(reset); break;
  }

  switch(model()) {
  case Model::VSSystem:     cpuM.coprocessors.append(&vssystem); cpuS.coprocessors.append(&vssystem); break;
  case Model::PlayChoice10: cpuM.coprocessors.append(&playchoice10.pc10cpu); break;
  case Model::FamicomBox:   cpuM.coprocessors.append(&famicombox); break;
  }

  scheduler.primary(Famicom::Model::VSSystem() && vssystem.gameCount == 1 ? cpuS : cpuM);

  if(!Famicom::Model::VSSystem() || vssystem.gameCount == 2) {
    controllerPortM1.power(0, ID::Port::Controller1);
    controllerPortM2.power(0, ID::Port::Controller2);
  }
  if(!Famicom::Model::VSSystem()) expansionPort.power();
  if(Famicom::Model::VSSystem()) {
    controllerPortS1.power(1, ID::Port::Controller1);
    controllerPortS2.power(1, ID::Port::Controller2);
  }

  switch(model()) {

  case Model::Famicom:
  case Model::FamicomBox: {
    controllerPortM1.connect(settings.controllerPort1);
    controllerPortM2.connect(settings.controllerPort2);
    expansionPort.connect(settings.expansionPort);
    break;
  }

  case Model::VSSystem: {
    auto side = cartridgeSlot[0].game.document.find("side");
    for(bool sideIndex : range(side.size())) {
      bool sideID = sideIndex + (2 - side.size());

      bool& swapControllers = sideID ? vssystem.swapControllersS : vssystem.swapControllersM;
      swapControllers = side(side.size() - 1)["controller/port"].integer() == 2;

      if(side(sideIndex)["ppu"]) {
        string device1 = side(sideIndex).find("controller(port=1)/device")(0).text();
        string device2 = side(sideIndex).find("controller(port=2)/device")(0).text();

        auto& controllerPort1 = sideID ? controllerPortS1 : controllerPortM1;
        auto& controllerPort2 = sideID ? controllerPortS2 : controllerPortM2;

        if(device1 == "gamepad") {
          controllerPort1.connect(ID::Device::Gamepad);
        } else if(device1 == "zapper") {
          controllerPort1.connect(ID::Device::Zapper);
        } else {
          controllerPort1.connect(ID::Device::None);
        }

        if(device2 == "gamepad") {
          controllerPort2.connect(ID::Device::Gamepad);
        } else if(device2 == "zapper") {
          controllerPort2.connect(ID::Device::Zapper);
        } else {
          controllerPort2.connect(ID::Device::None);
        }
      }
    }
    break;
  }

  case Model::PlayChoice10: {
    controllerPortM1.connect(ID::Device::Gamepad);
    controllerPortM2.connect(ID::Device::Gamepad);
    expansionPort.connect(settings.expansionPort);
    break;
  }

  }
}

}
