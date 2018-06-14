#include "../laevateinn.hpp"
unique_pointer<ConsoleWindow> consoleWindow;

#include "about.cpp"

ConsoleWindow::ConsoleWindow() {
  consoleWindow = this;

  layout.setMargin(5);
  setTitle({"Console - star-rod v", Emulator::ForkVersion});

  emulationMenu.setText("&Emulation");
  reloadCartridge.setText("Reload Cartridge").onActivate([&] {
    program->mediumQueue.append(program->mediumPaths(1));
    program->loadMedium(*emulator, emulator->media[0]);
    debugger->resetUsage();
  });
  powerCycle.setText("Power Cycle").onActivate([&] {
    emulator->power();
    print("System power cycled\n");
  });
  synchronizeAudio.setText("Synchronize Audio").setChecked(settings["Audio/Synchronize"].boolean()).onToggle([&] {
    settings["Audio/Synchronize"].setValue(synchronizeAudio.checked());
    audio->setBlocking(synchronizeAudio.checked());
  });
  muteAudio.setText("Mute Audio").setChecked(settings["Audio/Mute"].boolean()).onToggle([&] {
    settings["Audio/Mute"].setValue(muteAudio.checked());
    program->updateAudioEffects();
  });

  debugMenu.setText("&Debug");
  debugCPU.setText("CPU").setChecked(debugger->debug.cpu).onToggle([&] {
    debugger->debug.cpu = debugCPU.checked();
  });
  debugSMP.setText("SMP").setChecked(debugger->debug.smp).onToggle([&] {
    debugger->debug.smp = debugSMP.checked();
  });

  tracerMenu.setText("&Tracer");
  tracerEnable.setText("Enable").onToggle([&] {
    tracer->enable(tracerEnable.checked());
  });
  tracerMask.setText("Mask").setChecked(tracer->mask).onToggle([&] {
    tracer->mask = tracerMask.checked();
  });
  tracerMaskReset.setText("Reset Mask").onActivate([&] {
    tracer->resetMask();
    debugger->print("Tracer mask reset\n");
  });

  windowsMenu.setText("&Windows");
  windowsPresentation.setText("Video").onActivate([&] {
    presentation->setVisible();
    presentation->setFocused();
  });
  windowsCPUDebugger.setText("CPU Debugger").onActivate([&] {
    cpuDebugger->setVisible();
    cpuDebugger->setFocused();
  });
  windowsSMPDebugger.setText("SMP Debugger").onActivate([&] {
    smpDebugger->setVisible();
    smpDebugger->setFocused();
  });
  windowsMemoryEditor.setText("Memory Editor").onActivate([&] {
    memoryEditor->updateView();
    memoryEditor->setVisible();
    memoryEditor->setFocused();
  });
  windowsBreakpointEditor.setText("Breakpoint Editor").onActivate([&] {
    breakpointEditor->setVisible();
    breakpointEditor->setFocused();
  });
  windowsPropertiesViewer.setText("Properties Viewer").onActivate([&] {
    propertiesViewer->setVisible();
    propertiesViewer->setFocused();
  });
  windowsTileViewer.setText("Tile Viewer").onActivate([&] {
    tileViewer->setVisible();
    tileViewer->setFocused();
  });
  windowsBGViewer.setText("BG Viewer").onActivate([&] {
    bgViewer->setVisible();
    bgViewer->setFocused();
  });
  windowsPaletteViewer.setText("Palette Viewer").onActivate([&] {
    paletteViewer->setVisible();
    paletteViewer->setFocused();
  });

  stateMenu.setText("&State");
  saveStateMenu.setText("Save");
  saveSlot0.setText("Slot 0").onActivate([&] { program->saveState(0); });
  saveSlot1.setText("Slot 1").onActivate([&] { program->saveState(1); });
  saveSlot2.setText("Slot 2").onActivate([&] { program->saveState(2); });
  saveSlot3.setText("Slot 3").onActivate([&] { program->saveState(3); });
  saveSlot4.setText("Slot 4").onActivate([&] { program->saveState(4); });
  saveSlot5.setText("Slot 5").onActivate([&] { program->saveState(5); });
  saveSlot6.setText("Slot 6").onActivate([&] { program->saveState(6); });
  saveSlot7.setText("Slot 7").onActivate([&] { program->saveState(7); });
  saveSlot8.setText("Slot 8").onActivate([&] { program->saveState(8); });
  saveSlot9.setText("Slot 9").onActivate([&] { program->saveState(9); });
  loadStateMenu.setText("Load");
  loadSlot0.setText("Slot 0").onActivate([&] { program->loadState(0); });
  loadSlot1.setText("Slot 1").onActivate([&] { program->loadState(1); });
  loadSlot2.setText("Slot 2").onActivate([&] { program->loadState(2); });
  loadSlot3.setText("Slot 3").onActivate([&] { program->loadState(3); });
  loadSlot4.setText("Slot 4").onActivate([&] { program->loadState(4); });
  loadSlot5.setText("Slot 5").onActivate([&] { program->loadState(5); });
  loadSlot6.setText("Slot 6").onActivate([&] { program->loadState(6); });
  loadSlot7.setText("Slot 7").onActivate([&] { program->loadState(7); });
  loadSlot8.setText("Slot 8").onActivate([&] { program->loadState(8); });
  loadSlot9.setText("Slot 9").onActivate([&] { program->loadState(9); });

  helpMenu.setText("&Help");
  about.setText("About ...").onActivate([&] { aboutWindow->setVisible(); });

  runButton.setText("Run").onActivate([&] {
    if(debugger->paused) {
      print("\n");
      debugger->resume();
    } else {
      debugger->suspend();
    }
  });
  stepButton.setText("Step").onActivate([&] {
    debugger->flags.step = true;
    debugger->resume();
  });
  clearButton.setText("Clear").onActivate([&] {
    console.setText("");
  });

  console.setFont(Font().setFamily(Font::Mono));

  onClose([&] {
    program->quit();
  });

  setSize({640, 400});
  setAlignment({0.0, 1.0});
}

auto ConsoleWindow::print(const string& text) -> void {
  string output = console.text();
  output.append(text);
  console.setText(output);
  console.setCursor({INT_MAX, 0});
}
