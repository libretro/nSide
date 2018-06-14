struct CPUDebugger : Window {
  CPUDebugger();

  auto mirror(uint24 addr) -> uint24;
  auto read(uint24 addr) -> uint8;
  auto write(uint24 addr, uint8 data) -> void;

  auto opcodeLength(uint24 addr) -> uint;
  auto updateDisassembly() -> void;

  VerticalLayout layout{this};
    HorizontalLayout controlLayout{&layout, Size{~0, 0}};
      Button stepInto{&controlLayout, Size{80, 0}};
      Button stepNMI{&controlLayout, Size{40, 0}};
      Button stepIRQ{&controlLayout, Size{40, 0}};
      Widget spacer{&controlLayout, Size{~0, 0}};
      CheckLabel autoUpdate{&controlLayout, Size{0, 0}};
      Button update{&controlLayout, Size{80, 0}};
    TextEdit disassembly{&layout, Size{~0, ~0}};
    Button registers{&layout, Size{~0, 0}};

  uint24 opcodePC;
};

struct CPURegisterEditor : Window {
  CPURegisterEditor();

  auto loadRegisters() -> void;
  auto saveRegisters() -> void;

  VerticalLayout layout{this};
    HorizontalLayout primaryLayout{&layout, Size{~0, 0}};
      Label    regALabel{&primaryLayout, Size{0, 0}};
      LineEdit regAValue{&primaryLayout, Size{0, 0}};
      Label    regXLabel{&primaryLayout, Size{0, 0}};
      LineEdit regXValue{&primaryLayout, Size{0, 0}};
      Label    regYLabel{&primaryLayout, Size{0, 0}};
      LineEdit regYValue{&primaryLayout, Size{0, 0}};
      Label    regSLabel{&primaryLayout, Size{0, 0}};
      LineEdit regSValue{&primaryLayout, Size{0, 0}};
      Label    regDLabel{&primaryLayout, Size{0, 0}};
      LineEdit regDValue{&primaryLayout, Size{0, 0}};
      Label    regBLabel{&primaryLayout, Size{0, 0}};
      LineEdit regBValue{&primaryLayout, Size{0, 0}};
    HorizontalLayout secondaryLayout{&layout, Size{~0, 0}};
      CheckLabel flagN{&secondaryLayout, Size{0, 0}};
      CheckLabel flagV{&secondaryLayout, Size{0, 0}};
      CheckLabel flagM{&secondaryLayout, Size{0, 0}};
      CheckLabel flagX{&secondaryLayout, Size{0, 0}};
      CheckLabel flagD{&secondaryLayout, Size{0, 0}};
      CheckLabel flagI{&secondaryLayout, Size{0, 0}};
      CheckLabel flagZ{&secondaryLayout, Size{0, 0}};
      CheckLabel flagC{&secondaryLayout, Size{0, 0}};
    HorizontalLayout tertiaryLayout{&layout, Size{~0, 0}};
      CheckLabel flagE{&tertiaryLayout, Size{~0, 0}};
      Widget spacer{&tertiaryLayout, Size{0, 0}};
      Button update{&tertiaryLayout, Size{80, 0}};
};

extern unique_pointer<CPUDebugger> cpuDebugger;
extern unique_pointer<CPURegisterEditor> cpuRegisterEditor;
