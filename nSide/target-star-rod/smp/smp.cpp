#include "../laevateinn.hpp"
unique_pointer<SMPDebugger> smpDebugger;

#include "registers.cpp"

SMPDebugger::SMPDebugger() {
  smpDebugger = this;
  opcodePC = 0xffc0;

  layout.setMargin(5);
  setTitle("SMP Debugger");

  stepInto.setText("Step Into").onActivate([&] {
    debugger->flags.smp.stepInto = true;
    debugger->resume();
  });
  autoUpdate.setText("Auto");
  update.setText("Update").onActivate({ &SMPDebugger::updateDisassembly, this });
  disassembly.setFont(Font().setFamily(Font::Mono));
  registers.setFont(Font().setFamily(Font::Mono)).setText(" ").onActivate([&] {
    smpRegisterEditor->loadRegisters();
    smpRegisterEditor->setVisible();
  });

  setSize({350, 260});
  setAlignment({0.5, 0.0});
}

auto SMPDebugger::read(uint16 addr) -> uint8 {
  if((addr & 0xfff0) == 0x00f0) return ~0;  //$00f0-00ff  MMIO
  return SFC::smp.busRead(addr);
}

auto SMPDebugger::write(uint16 addr, uint8 data) -> void {
  if((addr & 0xfff0) == 0x00f0) return;  //$00f0-00ff  MMIO
  return SFC::smp.busWrite(addr, data);
}

auto SMPDebugger::opcodeLength(uint16 addr) -> uint {
  static uint lengthTable[256] = {
    0
  };
  return lengthTable[SFC::smp.busRead(addr)];
}

auto SMPDebugger::updateDisassembly() -> void {
  const int middle = 7;
  string line[middle * 2 + 1];

  bool p = debugger->apuUsage.data[opcodePC] & Usage::P;
  string text = SFC::smp.disassemble(opcodePC, p);
  line[middle] = { "> ", slice(text, 0, 30) };

  int addr = opcodePC;
  for(int o = middle - 1; o >= 0; o--) {
    for(int b = 1; b <= 3; b++) {
      if(addr - b >= 0 && (debugger->apuUsage.data[addr - b] & Usage::Exec)) {
        addr -= b;
        p = debugger->apuUsage.data[addr] & Usage::P;
        text = SFC::smp.disassemble(addr, p);
        line[o] = { "  ", slice(text, 0, 30) };
        break;
      }
    }
  }

  addr = opcodePC;
  for(int o = middle + 1; o <= middle * 2; o++) {
    for(int b = 1; b <= 3; b++) {
      if(addr - b <= 0xffff && (debugger->apuUsage.data[addr + b] & Usage::Exec)) {
        addr += b;
        p = debugger->apuUsage.data[addr] & Usage::P;
        text = SFC::smp.disassemble(addr, p);
        line[o] = { "  ", slice(text, 0, 30) };
        break;
      }
    }
  }

  string output;
  for(auto& n : line) {
    if(!n) output.append("  ...\n");
    else   output.append(n, "\n");
  }
  output.trimRight("\n");

  disassembly.setText(output);
  registers.setText({
    "YA:", hex(SFC::smp.r.ya.w, 4L),
    " A:", hex(SFC::smp.r.ya.byte.l, 2L), " X:", hex(SFC::smp.r.x, 2L),
    " Y:", hex(SFC::smp.r.ya.byte.h, 2L), " S:01", hex(SFC::smp.r.s, 2L), " ",
    SFC::smp.r.p.n ? "N" : "n", SFC::smp.r.p.v ? "V" : "v",
    SFC::smp.r.p.p ? "P" : "p", SFC::smp.r.p.b ? "B" : "b",
    SFC::smp.r.p.h ? "H" : "h", SFC::smp.r.p.i ? "I" : "i",
    SFC::smp.r.p.z ? "Z" : "z", SFC::smp.r.p.c ? "C" : "c",
  });
}
