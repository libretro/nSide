AdvancedSettings::AdvancedSettings(TabFrame* parent) : TabFrameItem(parent) {
  setIcon(Icon::Action::Settings);

  layout.setMargin(5);

  localeLabel.setFont(Font().setBold());
  localeSelect.onChange([&] {
    settings["UserInterface/Locale"].setValue(localeSelect.selected().text());
    locale.load(settings["UserInterface/Locale"].text());
    locale.refresh();
  });
  ComboButtonItem localeAuto;
  localeAuto.setText("Auto");
  if(settings["UserInterface/Locale"].text() == "Auto") localeAuto.setSelected();
  localeSelect.append(localeAuto);
  for(auto& filename : directory::files(locate("Locales/"), "*.bml")) {
    if(filename == "default.bml") continue;
    string locale = string{filename}.trimRight(".bml", 1L);
    ComboButtonItem item;
    item.setText(locale);
    localeSelect.append(item);
    if(settings["UserInterface/Locale"].text() == locale) item.setSelected();
  }

  driverLabel.setFont(Font().setBold());
  videoDriver.onChange([&] { settings["Video/Driver"].setValue(videoDriver.selected().text()); });
  for(auto& driver : Video::availableDrivers()) {
    ComboButtonItem item;
    item.setText(driver);
    videoDriver.append(item);
    if(settings["Video/Driver"].text() == driver) item.setSelected();
  }
  audioDriver.onChange([&] { settings["Audio/Driver"].setValue(audioDriver.selected().text()); });
  for(auto& driver : Audio::availableDrivers()) {
    ComboButtonItem item;
    item.setText(driver);
    audioDriver.append(item);
    if(settings["Audio/Driver"].text() == driver) item.setSelected();
  }
  inputDriver.onChange([&] { settings["Input/Driver"].setValue(inputDriver.selected().text()); });
  for(auto& driver : Input::availableDrivers()) {
    ComboButtonItem item;
    item.setText(driver);
    inputDriver.append(item);
    if(settings["Input/Driver"].text() == driver) item.setSelected();
  }

  libraryLabel.setFont(Font().setBold());
  libraryLocation.setEditable(false).setText(settings["Library/Location"].text());
  libraryChange.onActivate([&] {
    if(auto location = BrowserDialog().setTitle("Select Library Location").selectFolder()) {
      settings["Library/Location"].setValue(location);
      libraryLocation.setText(location);
    }
  });

  ignoreManifests.setChecked(settings["Library/IgnoreManifests"].boolean()).onToggle([&] {
    settings["Library/IgnoreManifests"].setValue(ignoreManifests.checked());
  });

  otherLabel.setText("Other").setFont(Font().setBold());
  autoSaveMemory.setChecked(settings["Emulation/AutoSaveMemory/Enable"].boolean()).onToggle([&] {
    settings["Emulation/AutoSaveMemory/Enable"].setValue(autoSaveMemory.checked());
  });
  recentList.setChecked(settings["Library/RecentList"].boolean()).onToggle([&] {
    if(!recentList.checked()) {
      string prompt = locale["Settings/Advanced/Other/RecentList/Clear"];
      string yes = locale["Settings/Advanced/Other/RecentList/Clear/Yes"];
      string no = locale["Settings/Advanced/Other/RecentList/Clear/No"];
      if(MessageDialog(prompt).setParent(*settingsManager).question({yes, no}) == yes) {
        for(uint index : range(10)) {
          settings[{"Recent/", index, "/Title"}].setValue("");
          settings[{"Recent/", index, "/Path"}].setValue("");
        }
      } else {
        recentList.setChecked(true);
      }
    }
    settings["Library/RecentList"].setValue(recentList.checked());
    presentation->updateRecentList();
  });
}

auto AdvancedSettings::refreshLocale() -> void {
  setText(locale["Settings/Advanced"]);

  localeLabel.setText("Locale:");

  driverLabel.setText(locale["Settings/Advanced/DriverSelection"]);
  videoLabel.setText(locale["Settings/Advanced/DriverSelection/Video"]);
  audioLabel.setText(locale["Settings/Advanced/DriverSelection/Audio"]);
  inputLabel.setText(locale["Settings/Advanced/DriverSelection/Input"]);

  libraryLabel.setText(locale["Settings/Advanced/GameLibrary"]);
  libraryPrefix.setText(locale["Settings/Advanced/GameLibrary/Location"]);
  libraryChange.setText(locale["Settings/Advanced/GameLibrary/Change..."]);
  ignoreManifests.setText(locale["Settings/Advanced/GameLibrary/IgnoreManifests"]);

  otherLabel.setText(locale["Settings/Advanced/Other"]);
  autoSaveMemory.setText(locale["Settings/Advanced/Other/AutoSaveMemory"]);
  recentList.setText(locale["Settings/Advanced/Other/RecentList"]);

  layout.setMargin(5);
}
