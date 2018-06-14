#include "../shimai.hpp"
#include "hotkeys.cpp"
unique_pointer<InputManager> inputManager;

//build mappings list from assignment string
auto InputMapping::bind() -> void {
  mappings.reset();

  auto andGroups = assignment.split("|");
  for(auto& lists : andGroups) {
    auto list = lists.split("&");
    vector<Mapping> andGroup;
    for(auto& item : list) {
      item.trimLeft("!", 1L);
      auto token = item.split("/");
      if(token.size() < 3) continue;  //skip invalid mappings

      uint64 id = token[0].natural();
      uint group = token[1].natural();
      uint input = token[2].natural();
      string qualifier = token(3, "None");

      Mapping mapping;
      for(auto& device : inputManager->devices) {
        if(id != device->id()) continue;

        mapping.device = device;
        mapping.group = group;
        mapping.input = input;
        mapping.qualifier = Qualifier::None;
        if(qualifier == "Lo") mapping.qualifier = Qualifier::Lo;
        if(qualifier == "Hi") mapping.qualifier = Qualifier::Hi;
        if(qualifier == "Rumble") mapping.qualifier = Qualifier::Rumble;
        break;
      }

      if(!mapping.device) continue;
      andGroup.append(mapping);
    }
    if(!andGroup) continue;
    mappings.append(andGroup);
  }

  settings[path].setValue(assignment);
}

//append new mapping to mappings list
auto InputMapping::bind(string mapping, Logic logic) -> void {
  auto andGroups = assignment.split("|");
  if(logic == Logic::AND) {
    //if the far-right AND group contains the mapping being assigned
    if(andGroups.right().split("&").find(mapping)) return;
  } else if(logic == Logic::OR) {
    //if any AND group consists exclusively of the mapping being assigned
    for(string& andGroup : andGroups) if(andGroup == mapping) return;
  }
  if(!assignment || assignment == "None") {
    //create new mapping
    assignment = mapping;
  } else {
    //add additional mapping
    assignment.append(logic == Logic::AND ? "&" : "|");
    assignment.append(mapping);
  }
  bind();
}

auto InputMapping::bind(shared_pointer<HID::Device> device, uint group, uint input, int16 newValue, Logic logic) -> bool {
  if(device->isNull() || (device->isKeyboard() && device->group(group).input(input).name() == "Escape")) {
    return unbind(), true;
  }

  string encoding = {"0x", hex(device->id()), "/", group, "/", input};

  if(isDigital()) {
    if((device->isKeyboard() && group == HID::Keyboard::GroupID::Button)
    || (device->isMouse() && group == HID::Mouse::GroupID::Button)
    || (device->isJoypad() && group == HID::Joypad::GroupID::Button)) {
      if(newValue) {
        return bind(encoding, logic), true;
      }
    }

    if((device->isJoypad() && group == HID::Joypad::GroupID::Axis)
    || (device->isJoypad() && group == HID::Joypad::GroupID::Hat)
    || (device->isJoypad() && group == HID::Joypad::GroupID::Trigger)) {
      if(newValue < -16384 && group != HID::Joypad::GroupID::Trigger) {  //triggers are always hi
        return bind({encoding, "/Lo"}, logic), true;
      }

      if(newValue > +16384) {
        return bind({encoding, "/Hi"}, logic), true;
      }
    }
  }

  if(isAnalog()) {
    if((device->isMouse() && group == HID::Mouse::GroupID::Axis)
    || (device->isJoypad() && group == HID::Joypad::GroupID::Axis)
    || (device->isJoypad() && group == HID::Joypad::GroupID::Hat)) {
      if(newValue < -16384 || newValue > +16384) {
        return bind(encoding, logic), true;
      }
    }
  }

  if(isRumble()) {
    if(device->isJoypad() && group == HID::Joypad::GroupID::Button) {
      if(newValue) {
        return bind({encoding, "/Rumble"}, logic), true;
      }
    }
  }

  return false;
}

auto InputMapping::poll() -> int16 {
  if(!mappings) return 0;

  if(isDigital()) {
    bool result = 0;

    for(auto& andGroup : mappings) for(auto& mapping : andGroup) {
      auto value = mapping.device->group(mapping.group).input(mapping.input).value();
      bool output;

      if(mapping.device->isKeyboard() && mapping.group == HID::Keyboard::GroupID::Button) output = value != 0;
      if(mapping.device->isMouse() && mapping.group == HID::Mouse::GroupID::Button) output = value != 0;
      if(mapping.device->isJoypad() && mapping.group == HID::Joypad::GroupID::Button) output = value != 0;
      if((mapping.device->isJoypad() && mapping.group == HID::Joypad::GroupID::Axis)
      || (mapping.device->isJoypad() && mapping.group == HID::Joypad::GroupID::Hat)
      || (mapping.device->isJoypad() && mapping.group == HID::Joypad::GroupID::Trigger)) {
        if(mapping.qualifier == Qualifier::Lo) output = value < -16384;
        if(mapping.qualifier == Qualifier::Hi) output = value > +16384;
      }

      result |= output;
    }

    return result;
  }

  if(isAnalog()) {
    int16 result = 0;

    for(auto& andGroup : mappings) for(auto& mapping : andGroup) {
      auto value = mapping.device->group(mapping.group).input(mapping.input).value();

      //logic does not apply to analog inputs ... always combinatorial
      if(mapping.device->isMouse() && mapping.group == HID::Mouse::GroupID::Axis) result += value;
      if(mapping.device->isJoypad() && mapping.group == HID::Joypad::GroupID::Axis) result += value >> 8;
      if(mapping.device->isJoypad() && mapping.group == HID::Joypad::GroupID::Hat) result += value < 0 ? -1 : value > 0 ? +1 : 0;
    }

    return result;
  }

  return 0;
}

auto InputMapping::rumble(bool enable) -> void {
  if(!mappings) return;
  for(auto& andGroup : mappings) for(auto& mapping : andGroup) {
    ::input->rumble(mapping.device->id(), enable);
  }
}

auto InputMapping::unbind() -> void {
  mappings.reset();
  assignment = "None";
  settings[path].setValue(assignment);
}

//

InputManager::InputManager() {
  inputManager = this;
  frequency = max(1u, settings["Input/Frequency"].natural());

  for(auto& emulator : program->emulators) {
    InputEmulator inputEmulator;
    inputEmulator.interface = emulator;
    inputEmulator.name = emulator->information.name;
    for(auto& port : emulator->ports) {
      InputPort inputPort{port.id, port.name};
      for(auto& device : port.devices) {
        InputDevice inputDevice{device.id, device.name};
        for(auto& input : device.inputs) {
          InputMapping inputMapping;
          inputMapping.name = input.name;
          inputMapping.type = input.type;

          inputMapping.path = string{inputEmulator.name, "/", inputPort.name, "/", inputDevice.name, "/", inputMapping.name}.replace(" ", "");
          inputMapping.assignment = settings(inputMapping.path).text();
          inputMapping.bind();

          inputDevice.mappings.append(inputMapping);
        }
        inputPort.devices.append(inputDevice);
      }
      inputEmulator.ports.append(move(inputPort));
    }
    emulators.append(move(inputEmulator));
  }

  appendHotkeys();
}

//Emulator::Interface::inputPoll() needs to call into InputManager::InputEmulator
//this function is calling during Program::loadMedium() to link the two together
auto InputManager::bind(Emulator::Interface* interface) -> void {
  this->emulator = nullptr;
  for(auto& emulator : emulators) {
    if(emulator.interface == interface) {
      this->emulator = &emulator;
    }
  }
  assert(this->emulator != nullptr);
}

auto InputManager::bind() -> void {
  for(auto& emulator : emulators) {
    for(auto& port : emulator.ports) {
      for(auto& device : port.devices) {
        for(auto& mapping : device.mappings) {
          mapping.bind();
        }
      }
    }
  }

  for(auto& hotkey : hotkeys) {
    hotkey->bind();
  }
}

auto InputManager::poll() -> void {
  //polling actual hardware is very time-consuming: skip call if poll was called too recently
  auto thisPoll = chrono::millisecond();
  if(thisPoll - lastPoll < frequency) return;
  lastPoll = thisPoll;

  auto devices = input->poll();
  bool changed = devices.size() != this->devices.size();
  if(!changed) {
    for(auto n : range(devices)) {
      changed = devices[n] != this->devices[n];
      if(changed) break;
    }
  }
  if(changed) {
    this->devices = devices;
    bind();
  }
}

auto InputManager::onChange(shared_pointer<HID::Device> device, uint group, uint input, int16_t oldValue, int16_t newValue) -> void {
//if(settingsManager->focused()) {
//  settingsManager->input.inputEvent(device, group, input, newValue);
//  settingsManager->hotkeys.inputEvent(device, group, input, newValue);
//}
}

auto InputManager::quit() -> void {
  emulators.reset();
  hotkeys.reset();
}

auto InputManager::mapping(uint port, uint device, uint input) -> maybe<InputMapping&> {
  if(!emulator) return nothing;
  for(auto& inputPort : emulator->ports) {
    if(inputPort.id != port) continue;
    for(auto& inputDevice : inputPort.devices) {
      if(inputDevice.id != device) continue;
      if(input >= inputDevice.mappings.size()) return nothing;
      return inputDevice.mappings[input];
    }
  }
  return nothing;
}

auto InputManager::findMouse() -> shared_pointer<HID::Device> {
  for(auto& device : devices) {
    if(device->isMouse()) return device;
  }
  return {};
}
