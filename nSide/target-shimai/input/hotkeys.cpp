auto InputHotkey::pollIndex() -> maybe<uint> {
  if(!mappings) return nothing;

  for(uint id : range(mappings.size())) {
    bool result = 1;
    for(auto& mapping : mappings[id]) {
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

      result &= output;
    }
    if(result) return id;
  }

  return nothing;
}

//

auto InputManager::appendHotkeys() -> void {
  static int quickStateSlot = 0;

  { auto hotkey = new InputHotkey;
    hotkey->name = "Toggle Fullscreen";
    hotkey->press = [] {
      presentation->toggleFullScreen();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Toggle Mouse Capture";
    hotkey->press = [] {
      input->acquired() ? input->release() : input->acquire();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Save Quick State";
    hotkey->press = [] {
      program->saveState(quickStateSlot);
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Load Quick State";
    hotkey->press = [&] {
      program->loadState(quickStateSlot);
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Decrement Quick State";
    hotkey->press = [&] {
      if(--quickStateSlot < 0) quickStateSlot = 9;
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Increment Quick State";
    hotkey->press = [&] {
      if(++quickStateSlot > 9) quickStateSlot = 0;
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Pause Emulation";
    hotkey->press = [] {
      program->pause = !program->pause;
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Next Frame";
    hotkey->press = [] {
      if(!::emulator) return;

      program->pause = false;
      ::emulator->run();
      program->pause = true;
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Fast Forward";
    hotkey->press = [] {
      video->setBlocking(false);
      audio->setBlocking(false);
    };
    hotkey->release = [] {
      video->setBlocking(settings["Video/Synchronize"].boolean());
      audio->setBlocking(settings["Audio/Synchronize"].boolean());
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Soft Reset";
    hotkey->press = [] {
      program->softReset();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Power Cycle";
    hotkey->press = [] {
      program->powerCycle();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Rotate Display";
    hotkey->press = [] {
      program->rotateDisplay();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Save Screenshot";
    hotkey->press = [] {
      if(!::emulator) return;

      auto time = chrono::timestamp();
      string filename = {
        "screenshot-",
        chrono::local::datetime(time).replace(":", ";").replace(" ", "-"),
        ".png"
      };
      Emulator::video.screenshot({program->mediumPaths.right(), filename});
    };
    hotkeys.append(hotkey);
  }

  for(auto& hotkey : hotkeys) {
    hotkey->path = string{"Hotkey/", hotkey->name}.replace(" ", "");
    hotkey->assignment = settings(hotkey->path).text();
    hotkey->bind();
  }
}

auto InputManager::pollHotkeys() -> void {
  if(!program->focused() && !settings["Input/FocusLoss/AllowInput"].boolean()) return;

  static InputHotkey* activeHotkey = nullptr;
  InputHotkey* selectedHotkey = nullptr;
  uint maxInputs = 0;
  for(auto& hotkey : hotkeys) {
    maybe<uint> index = hotkey->pollIndex();
    if(!index) continue;
    if(hotkey->mappings[index()].size() > maxInputs) {
      maxInputs = hotkey->mappings[index()].size();
      selectedHotkey = hotkey;
    }
  }
  if(activeHotkey != selectedHotkey) {
    if(activeHotkey && activeHotkey->release) activeHotkey->release();
    activeHotkey = selectedHotkey;
    if(activeHotkey && activeHotkey->press) activeHotkey->press();
  }

  static shared_pointer<HID::Device> keyboard = nullptr;
  if(!keyboard) {
    for(auto& device : inputManager->devices) {
      if(device->isKeyboard()) keyboard = device;
    }
  }

  //Unload/Quit hotkey
  //Because stock higan and nSide don't have this hotkey and yield to the OS's
  //global exit hotkey (Alt+F4 on Windows, etc.), which doesn't work in
  //fullscreen. Key code 0 represents Escape in all of ruby's input drivers.
  static bool previousEsc = 0;
  auto esc = keyboard->group(0).input(0).value();
  if(previousEsc == 0 && esc == 1) {
    if(::emulator) program->unloadMedium();
    else           program->quit();
  }
  previousEsc = esc;

  if(settings["Input/Driver"].text() == "Windows") {
    static shared_pointer<HID::Device> xinput1 = nullptr;
    if(!xinput1) {
      for(auto& device : inputManager->devices) {
        if(device->isJoypad() && device->id() == 0x1'045e'028e) xinput1 = device;
      }
    }

    if(xinput1) {
      auto guide = xinput1->group(HID::Joypad::GroupID::Button).input(10).value();
      if(guide == 1 && ::emulator) program->unloadMedium();
    }
  }
}
