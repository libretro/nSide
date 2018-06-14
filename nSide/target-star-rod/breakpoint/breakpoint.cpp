#include "../laevateinn.hpp"
unique_pointer<BreakpointEditor> breakpointEditor;

BreakpointEntry::BreakpointEntry() {
  static uint id = 1;

  enable.setText({ "#", id++ }).onToggle([&] {
    bool flag = !enable.checked();
    addr.setEnabled(flag);
    data.setEnabled(flag);
    type.setEnabled(flag);
    source.setEnabled(flag);
    breakpointEditor->synchronize();
  });

  addr.setFont(Font().setFamily(Font::Mono));
  data.setFont(Font().setFamily(Font::Mono));
  type.append(ComboButtonItem().setText("Read"));
  type.append(ComboButtonItem().setText("Write"));
  type.append(ComboButtonItem().setText("Exec"));
  source.append(ComboButtonItem().setText("CPU"));
  source.append(ComboButtonItem().setText("SMP"));
  source.append(ComboButtonItem().setText("VRAM"));
  source.append(ComboButtonItem().setText("OAM"));
  source.append(ComboButtonItem().setText("CGRAM"));
}

BreakpointEditor::BreakpointEditor() {
  breakpointEditor = this;

  layout.setMargin(5);
  setTitle("Breakpoint Editor");

  for(auto& bp : breakpointEntry) layout.append(bp, {0, 0});

  setGeometry({{128, 128}, layout.minimumSize()});
  synchronize();
}

//enable checkbox toggled on one of the five BreakpointEntry items:
//cache settings to decrease testing overhead whilst debugging
auto BreakpointEditor::synchronize() -> void {
  breakpoint.reset();
  uint id = 0;
  for(auto &entry : breakpointEntry) {
    id++;
    if(!entry.enable.checked()) continue;
    Breakpoint bp;
    bp.id = id;
    bp.compare = entry.data.text().length() > 0;
    bp.addr = entry.addr.text().hex();
    bp.data = entry.data.text().hex();
    bp.type = entry.type.selected().offset();
    bp.source = entry.source.selected().offset();
    breakpoint.append(bp);
  }

  breakpointReadCPU.reset();
  breakpointWriteCPU.reset();
  breakpointExecCPU.reset();

  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Read && bp.source == Breakpoint::CPU) breakpointReadCPU.append(bp);
  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Write && bp.source == Breakpoint::CPU) breakpointWriteCPU.append(bp);
  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Exec && bp.source == Breakpoint::CPU) breakpointExecCPU.append(bp);

  for(auto &bp : breakpointReadCPU) bp.addr = cpuDebugger->mirror(bp.addr);
  for(auto &bp : breakpointWriteCPU) bp.addr = cpuDebugger->mirror(bp.addr);
  for(auto &bp : breakpointExecCPU) bp.addr = cpuDebugger->mirror(bp.addr);

  breakpointReadSMP.reset();
  breakpointWriteSMP.reset();
  breakpointExecSMP.reset();

  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Read && bp.source == Breakpoint::SMP) breakpointReadSMP.append(bp);
  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Write && bp.source == Breakpoint::SMP) breakpointWriteSMP.append(bp);
  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Exec && bp.source == Breakpoint::SMP) breakpointExecSMP.append(bp);

  breakpointReadVRAM.reset();
  breakpointWriteVRAM.reset();

  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Read && bp.source == Breakpoint::VRAM) breakpointReadVRAM.append(bp);
  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Write && bp.source == Breakpoint::VRAM) breakpointWriteVRAM.append(bp);

  breakpointReadOAM.reset();
  breakpointWriteOAM.reset();

  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Read && bp.source == Breakpoint::OAM) breakpointReadOAM.append(bp);
  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Write && bp.source == Breakpoint::OAM) breakpointWriteOAM.append(bp);

  breakpointReadCGRAM.reset();
  breakpointWriteCGRAM.reset();

  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Read && bp.source == Breakpoint::CGRAM) breakpointReadCGRAM.append(bp);
  for(auto &bp : breakpoint) if(bp.type == Breakpoint::Write && bp.source == Breakpoint::CGRAM) breakpointWriteCGRAM.append(bp);
}

//S-CPU
//=====

auto BreakpointEditor::testReadCPU(uint24 addr) -> bool {
  addr = cpuDebugger->mirror(addr);
  for(auto &bp : breakpointReadCPU) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != cpuDebugger->read(addr)) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testWriteCPU(uint24 addr, uint8 data) -> bool {
  addr = cpuDebugger->mirror(addr);
  for(auto &bp : breakpointWriteCPU) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != data) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testExecCPU(uint24 addr) -> bool {
  addr = cpuDebugger->mirror(addr);
  for(auto &bp : breakpointExecCPU) {
    if(bp.addr == addr) {
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

//S-SMP
//=====

auto BreakpointEditor::testReadSMP(uint16 addr) -> bool {
  for(auto &bp : breakpointReadSMP) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != smpDebugger->read(addr)) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testWriteSMP(uint16 addr, uint8 data) -> bool {
  for(auto &bp : breakpointWriteSMP) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != data) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testExecSMP(uint16 addr) -> bool {
  for(auto &bp : breakpointExecSMP) {
    if(bp.addr == addr) {
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

//S-PPU
//=====

auto BreakpointEditor::testReadVRAM(uint16 addr) -> bool {
  for(auto &bp : breakpointReadVRAM) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != SFC::ppu.vram[addr]) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testWriteVRAM(uint16 addr, uint8 data) -> bool {
  for(auto &bp : breakpointWriteVRAM) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != data) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testReadOAM(uint16 addr) -> bool {
  for(auto &bp : breakpointReadOAM) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != SFC::ppu.obj.oam.read(addr)) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testWriteOAM(uint16 addr, uint8 data) -> bool {
  for(auto &bp : breakpointWriteOAM) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != data) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testReadCGRAM(uint16 addr) -> bool {
  for(auto &bp : breakpointReadCGRAM) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != SFC::ppu.screen.cgram[addr]) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}

auto BreakpointEditor::testWriteCGRAM(uint16 addr, uint8 data) -> bool {
  for(auto &bp : breakpointWriteCGRAM) {
    if(bp.addr == addr) {
      if(bp.compare && bp.data != data) continue;
      debugger->print("Breakpoint #", bp.id, " hit\n");
      return true;
    }
  }
  return false;
}
