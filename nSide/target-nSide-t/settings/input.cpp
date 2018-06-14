InputSettings::InputSettings(TabFrame* parent) : TabFrameItem(parent) {
  setIcon(Icon::Device::Joypad);

  layout.setMargin(5);
  pauseEmulation.setChecked(settings["Input/FocusLoss/Pause"].boolean()).onToggle([&] {
    settings["Input/FocusLoss/Pause"].setValue(pauseEmulation.checked());
    allowInput.setEnabled(!pauseEmulation.checked());
  }).doToggle();
  allowInput.setChecked(settings["Input/FocusLoss/AllowInput"].boolean()).onToggle([&] {
    settings["Input/FocusLoss/AllowInput"].setValue(allowInput.checked());
  });
  emulatorList.reset();
  for(auto& emulator : inputManager->emulators) {
    emulatorList.append(ComboButtonItem().setText(emulator.name));
  }
  emulatorList.onChange([&] { reloadPorts(); });
  portList.onChange([&] { reloadDevices(); });
  deviceList.onChange([&] { reloadMappings(); });
  mappingList.onActivate([&] { assignMapping(); });
  mappingList.onChange([&] { updateControls(); });
  assignMouse1.setVisible(false).onActivate([&] { assignMouseInput(0); });
  assignMouse2.setVisible(false).onActivate([&] { assignMouseInput(1); });
  assignMouse3.setVisible(false).onActivate([&] { assignMouseInput(2); });
  resetButton.onActivate([&] {
    string prompt = locale["Settings/Input/ResetConfirmation"];
    string yes = locale["Settings/Input/ResetConfirmation/Yes"];
    string no = locale["Settings/Input/ResetConfirmation/No"];
    if(MessageDialog(prompt).setParent(*settingsManager).question({yes, no}) == yes) {
      for(auto& mapping : activeDevice().mappings) mapping.unbind();
      refreshMappings();
    }
  });
  eraseButton.onActivate([&] {
    if(auto mapping = mappingList.selected()) {
      activeDevice().mappings[mapping.offset()].unbind();
      refreshMappings();
    }
  });

  reloadPorts();
}

auto InputSettings::refreshLocale() -> void {
  setText(locale["Settings/Input"]);

  focusLabel.setText(locale["Settings/Input/WhenFocusIsLost"]);
  pauseEmulation.setText(locale["Settings/Input/WhenFocusIsLost/PauseEmulation"]);
  allowInput.setText(locale["Settings/Input/WhenFocusIsLost/AllowInput"]);

  resetButton.setText(locale["Settings/Input/Reset"]);
  eraseButton.setText(locale["Settings/Input/Erase"]);
}

auto InputSettings::updateControls() -> void {
  eraseButton.setEnabled((bool)mappingList.selected());
  assignMouse1.setVisible(false);
  assignMouse2.setVisible(false);
  assignMouse3.setVisible(false);

  if(auto mapping = mappingList.selected()) {
    auto& input = activeDevice().mappings[mapping.offset()];

    if(input.isDigital()) {
      assignMouse1.setVisible().setText(locale["Settings/Input/MouseLeft"]);
      assignMouse2.setVisible().setText(locale["Settings/Input/MouseMiddle"]);
      assignMouse3.setVisible().setText(locale["Settings/Input/MouseRight"]);
    } else if(input.isAnalog()) {
      assignMouse1.setVisible().setText(locale["Settings/Input/MouseXAxis"]);
      assignMouse2.setVisible().setText(locale["Settings/Input/MouseYAxis"]);
    }
  }
}

auto InputSettings::activeEmulator() -> InputEmulator& {
  return inputManager->emulators[emulatorList.selected().offset()];
}

auto InputSettings::activePort() -> InputPort& {
  return activeEmulator().ports[portList.selected().offset()];
}

auto InputSettings::activeDevice() -> InputDevice& {
  auto index = deviceList.selected().property("index").natural();
  return activePort().devices[index];
}

auto InputSettings::reloadPorts() -> void {
  portList.reset();
  for(auto& port : activeEmulator().ports) {
    portList.append(ComboButtonItem().setText(port.name));
  }
  reloadDevices();
}

auto InputSettings::reloadDevices() -> void {
  deviceList.reset();
  for(auto n : range(activePort().devices)) {
    auto& device = activePort().devices[n];
    if(!device.mappings) continue;  //do not display devices that have no configurable inputs
    deviceList.append(ComboButtonItem().setText(device.name).setProperty("index", n));
  }
  reloadMappings();
}

auto InputSettings::reloadMappings() -> void {
  eraseButton.setEnabled(false);
  mappingList.reset();
  mappingList.append(TableViewHeader().setVisible()
    .append(TableViewColumn().setText(locale["Settings/Input/Name"]))
    .append(TableViewColumn().setText(locale["Settings/Input/Mapping"]).setExpandable())
  );
  for(auto& mapping : activeDevice().mappings) {
    mappingList.append(TableViewItem()
      .append(TableViewCell().setText(mapping.name))
      .append(TableViewCell())
    );
  }
  refreshMappings();
}

auto InputSettings::refreshMappings() -> void {
  uint index = 0;
  for(auto& mapping : activeDevice().mappings) {
    mappingList.item(index++).cell(1).setText(mapping.displayName());
  }
  mappingList.resizeColumns();
}

auto InputSettings::assignMapping() -> void {
  inputManager->poll();  //clear any pending events first

  if(auto mapping = mappingList.selected()) {
    activeMapping = &activeDevice().mappings[mapping.offset()];
    settingsManager->layout.setEnabled(false);
    settingsManager->statusBar.setText(locale["Settings/Input/PressKey"].replace("%s", activeMapping->name));
  }
}

auto InputSettings::assignMouseInput(uint id) -> void {
  if(auto mouse = inputManager->findMouse()) {
    if(auto mapping = mappingList.selected()) {
      activeMapping = &activeDevice().mappings[mapping.offset()];

      if(activeMapping->isDigital()) {
        return inputEvent(mouse, HID::Mouse::GroupID::Button, id, 1, true);  //oldValue wasn't used
      } else if(activeMapping->isAnalog()) {
        return inputEvent(mouse, HID::Mouse::GroupID::Axis, id, +32767, true);  //oldValue wasn't used
      }
    }
  }
}

auto InputSettings::inputEvent(shared_pointer<HID::Device> device, uint group, uint input, int16 newValue, bool allowMouseInput) -> void {
  if(!activeMapping) return;
  if(device->isMouse() && !allowMouseInput) return;

  if(activeMapping->bind(device, group, input, newValue, InputMapping::Logic::OR)) {
    activeMapping = nullptr;
    settingsManager->statusBar.setText(locale["Settings/Input/MappingAssigned"]);
    refreshMappings();
    timer.onActivate([&] {
      timer.setEnabled(false);
      settingsManager->statusBar.setText();
      settingsManager->layout.setEnabled();
    }).setInterval(200).setEnabled();
  }
}
