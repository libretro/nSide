struct ConsoleWindow : Window {
  ConsoleWindow();

  auto print(const string& text) -> void;

  MenuBar menuBar{this};
    Menu emulationMenu{&menuBar};
      MenuItem reloadCartridge{&emulationMenu};
      MenuItem powerCycle{&emulationMenu};
      MenuSeparator menuEmulationSeparator{&emulationMenu};
      MenuCheckItem synchronizeAudio{&emulationMenu};
      MenuCheckItem muteAudio{&emulationMenu};
    Menu debugMenu{&menuBar};
      MenuCheckItem debugCPU{&debugMenu};
      MenuCheckItem debugSMP{&debugMenu};
    Menu tracerMenu{&menuBar};
      MenuCheckItem tracerEnable{&tracerMenu};
      MenuCheckItem tracerMask{&tracerMenu};
      MenuItem tracerMaskReset{&tracerMenu};
    Menu windowsMenu{&menuBar};
      MenuItem windowsPresentation{&windowsMenu};
      MenuSeparator windowsSeparator1{&windowsMenu};
      MenuItem windowsCPUDebugger{&windowsMenu};
      MenuItem windowsSMPDebugger{&windowsMenu};
      MenuSeparator windowsSeparator2{&windowsMenu};
      MenuItem windowsMemoryEditor{&windowsMenu};
      MenuItem windowsBreakpointEditor{&windowsMenu};
      MenuItem windowsPropertiesViewer{&windowsMenu};
      MenuItem windowsTileViewer{&windowsMenu};
      MenuItem windowsBGViewer{&windowsMenu};
      MenuItem windowsPaletteViewer{&windowsMenu};
    Menu stateMenu{&menuBar};
      Menu saveStateMenu{&stateMenu};
        MenuItem saveSlot0{&saveStateMenu};
        MenuItem saveSlot1{&saveStateMenu};
        MenuItem saveSlot2{&saveStateMenu};
        MenuItem saveSlot3{&saveStateMenu};
        MenuItem saveSlot4{&saveStateMenu};
        MenuItem saveSlot5{&saveStateMenu};
        MenuItem saveSlot6{&saveStateMenu};
        MenuItem saveSlot7{&saveStateMenu};
        MenuItem saveSlot8{&saveStateMenu};
        MenuItem saveSlot9{&saveStateMenu};
      Menu loadStateMenu{&stateMenu};
        MenuItem loadSlot0{&loadStateMenu};
        MenuItem loadSlot1{&loadStateMenu};
        MenuItem loadSlot2{&loadStateMenu};
        MenuItem loadSlot3{&loadStateMenu};
        MenuItem loadSlot4{&loadStateMenu};
        MenuItem loadSlot5{&loadStateMenu};
        MenuItem loadSlot6{&loadStateMenu};
        MenuItem loadSlot7{&loadStateMenu};
        MenuItem loadSlot8{&loadStateMenu};
        MenuItem loadSlot9{&loadStateMenu};
    Menu helpMenu{&menuBar};
      MenuItem about{&helpMenu};

  VerticalLayout layout{this};
    HorizontalLayout commandLayout{&layout, Size{~0, 0}, 5};
      Button runButton{&commandLayout, Size{80, ~0}, 5};
      Button stepButton{&commandLayout, Size{80, ~0}, 5};
      Widget spacer{&commandLayout, Size{~0, 0}};
      Button clearButton{&commandLayout, Size{80, ~0}};
    TextEdit console{&layout, Size{~0, ~0}};
};

struct AboutWindow : Window {
  AboutWindow();

  VerticalLayout layout{this};
    Canvas canvas;
    Label title;
    Label version;
    Label website;
};

extern unique_pointer<ConsoleWindow> consoleWindow;
extern unique_pointer<AboutWindow> aboutWindow;
