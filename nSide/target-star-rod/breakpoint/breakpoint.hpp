struct BreakpointEntry : HorizontalLayout {
  BreakpointEntry();

  CheckLabel enable{this, Size{0, 0}};
  LineEdit addr{this, Size{50, 0}};
  LineEdit data{this, Size{25, 0}};
  ComboButton type{this, Size{0, 0}};
  ComboButton source{this, Size{0, 0}};
};

struct BreakpointEditor : Window {
  BreakpointEditor();

  VerticalLayout layout{this};
  BreakpointEntry breakpointEntry[8];

  struct Breakpoint {
    enum : uint { Read, Write, Exec };
    enum : uint { CPU, SMP, VRAM, OAM, CGRAM };
    uint id;
    bool compare;
    uint addr;
    uint data;
    uint type;
    uint source;
  };

  vector<Breakpoint> breakpoint;

  vector<Breakpoint> breakpointReadCPU;
  vector<Breakpoint> breakpointWriteCPU;
  vector<Breakpoint> breakpointExecCPU;

  vector<Breakpoint> breakpointReadSMP;
  vector<Breakpoint> breakpointWriteSMP;
  vector<Breakpoint> breakpointExecSMP;

  vector<Breakpoint> breakpointReadVRAM;
  vector<Breakpoint> breakpointWriteVRAM;

  vector<Breakpoint> breakpointReadOAM;
  vector<Breakpoint> breakpointWriteOAM;

  vector<Breakpoint> breakpointReadCGRAM;
  vector<Breakpoint> breakpointWriteCGRAM;

  auto synchronize() -> void;

  auto testReadCPU(uint24 addr) -> bool;
  auto testWriteCPU(uint24 addr, uint8 data) -> bool;
  auto testExecCPU(uint24 addr) -> bool;

  auto testReadSMP(uint16 addr) -> bool;
  auto testWriteSMP(uint16 addr, uint8 data) -> bool;
  auto testExecSMP(uint16 addr) -> bool;

  auto testReadVRAM(uint16 addr) -> bool;
  auto testWriteVRAM(uint16 addr, uint8 data) -> bool;

  auto testReadOAM(uint16 addr) -> bool;
  auto testWriteOAM(uint16 addr, uint8 data) -> bool;

  auto testReadCGRAM(uint16 addr) -> bool;
  auto testWriteCGRAM(uint16 addr, uint8 data) -> bool;
};

extern unique_pointer<BreakpointEditor> breakpointEditor;
