AudioSettings::AudioSettings(TabFrame* parent) : TabFrameItem(parent) {
  setIcon(Icon::Device::Speaker);

  layout.setMargin(5);

  driverLabel.setFont(Font().setBold());

  deviceList.onChange([&] {
    settings["Audio/Device"].setValue(deviceList.selected().text());
    program->initializeAudioDriver();
    updateDevice();
  });

  for(auto& device : audio->availableDevices()) {
    deviceList.append(ComboButtonItem().setText(device));
    if(device == settings["Audio/Device"].text()) {
      deviceList.item(deviceList.itemCount() - 1).setSelected();
    }
  }

  frequencyList.onChange([&] {
    settings["Audio/Frequency"].setValue(frequencyList.selected().text());
    program->updateAudioDriver();
  });

  latencyList.onChange([&] {
    settings["Audio/Latency"].setValue(latencyList.selected().text());
    program->updateAudioDriver();
  });

  exclusiveMode.setChecked(settings["Audio/Exclusive"].boolean()).onToggle([&] {
    settings["Audio/Exclusive"].setValue(exclusiveMode.checked());
    program->updateAudioDriver();
  });

  effectsLabel.setFont(Font().setBold());

  volumeValue.setAlignment(0.5);
  volumeSlider.setLength(201).setPosition(settings["Audio/Volume"].natural()).onChange([&] { updateEffects(); });

  balanceValue.setAlignment(0.5);
  balanceSlider.setLength(101).setPosition(settings["Audio/Balance"].natural()).onChange([&] { updateEffects(); });

  reverbEnable.setChecked(settings["Audio/Reverb/Enable"].boolean()).onToggle([&] { updateEffects(); });

  updateDevice();
  updateEffects(true);
}

auto AudioSettings::refreshLocale() -> void {
  setText(locale["Settings/Audio"]);

  driverLabel.setText(locale["Settings/Audio/Driver"]);
  deviceLabel.setText(locale["Settings/Audio/Driver/Device"]);
  frequencyLabel.setText(locale["Settings/Audio/Driver/Frequency"]);
  latencyLabel.setText(locale["Settings/Audio/Driver/Latency"]);
  exclusiveMode.setText(locale["Settings/Audio/Driver/ExclusiveMode"]);

  effectsLabel.setText(locale["Settings/Audio/Effects"]);
  volumeLabel.setText(locale["Settings/Audio/Effects/Volume"]);
  balanceLabel.setText(locale["Settings/Audio/Effects/Balance"]);
  reverbEnable.setText(locale["Settings/Audio/Effects/Reverb"]);
}

auto AudioSettings::updateDevice() -> void {
  frequencyList.reset();
  for(auto& frequency : audio->availableFrequencies()) {
    frequencyList.append(ComboButtonItem().setText((uint)frequency));
    if(frequency == settings["Audio/Frequency"].real()) {
      frequencyList.item(frequencyList.itemCount() - 1).setSelected();
    }
  }

  latencyList.reset();
  for(auto& latency : audio->availableLatencies()) {
    latencyList.append(ComboButtonItem().setText(latency));
    if(latency == settings["Audio/Latency"].natural()) {
      latencyList.item(latencyList.itemCount() - 1).setSelected();
    }
  }
}

auto AudioSettings::updateEffects(bool initializing) -> void {
  settings["Audio/Volume"].setValue(volumeSlider.position());
  volumeValue.setText({volumeSlider.position(), "%"});

  settings["Audio/Balance"].setValue(balanceSlider.position());
  balanceValue.setText({balanceSlider.position(), "%"});

  settings["Audio/Reverb/Enable"].setValue(reverbEnable.checked());

  if(!initializing) program->updateAudioEffects();
}
