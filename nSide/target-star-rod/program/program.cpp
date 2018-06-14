#include "../laevateinn.hpp"
#include "interface.cpp"
#include "medium.cpp"
#include "state.cpp"
#include "utility.cpp"
unique_pointer<Program> program;

Program::Program(string_vector args) {
  program = this;

  Emulator::platform = this;
  emulator = new SFC::Interface;

  new Settings;
  new Debugger;
  new Tracer;
  new ConsoleWindow;
  new AboutWindow;
  new Presentation;
  new CPUDebugger;
  new CPURegisterEditor;
  new SMPDebugger;
  new SMPRegisterEditor;
  new MemoryEditor;
  new BreakpointEditor;
  new PropertiesViewer;
  new TileViewer;
  new BGViewer;
  new PaletteViewer;

  higan_settings = BML::unserialize(string::read(locateHigan("settings.bml")));

  initializeVideoDriver();
  initializeAudioDriver();
  initializeInputDriver();

  args.takeLeft();  //ignore program location in argument parsing
  for(auto& argument : args) {
    if(directory::exists(argument.split("|", 1L).right())) {
      if(!argument.transform("\\", "/").endsWith("/")) argument.append("/");
      mediumQueue.append(argument);
    }
  }
  loadMedium(*emulator, emulator->media[0]);

  consoleWindow->setVisible();
  presentation->setVisible();
  consoleWindow->setFocused();
  presentation->drawSplashScreen();

  cpuDebugger->updateDisassembly();
  smpDebugger->updateDisassembly();
  memoryEditor->selectSource();
  propertiesViewer->updateProperties();
  tileViewer->updateTiles();
  bgViewer->updateTiles();
  paletteViewer->updateColors();

  Application::onMain({&Program::main, this});
}

auto Program::main() -> void {
  debugger->run();
}

auto Program::quit() -> void {
  unloadMedium();
  delete emulator;
  settings.quit();
  Application::quit();
}
