#include "../laevateinn.hpp"
unique_pointer<CPUDebugger> cpuDebugger;

#include "registers.cpp"

CPUDebugger::CPUDebugger() {
  cpuDebugger = this;
  opcodePC = 0x008000;

  layout.setMargin(5);
  setTitle("CPU Debugger");

  stepInto.setText("Step Into").onActivate([&] {
    debugger->flags.cpu.stepInto = true;
    debugger->resume();
  });
  stepNMI.setText("NMI").onActivate([&] {
    debugger->flags.cpu.nmi = true;
    debugger->resume();
  });
  stepIRQ.setText("IRQ").onActivate([&] {
    debugger->flags.cpu.irq = true;
    debugger->resume();
  });
  autoUpdate.setText("Auto");
  update.setText("Update").onActivate({ &CPUDebugger::updateDisassembly, this });
  disassembly.setFont(Font().setFamily(Font::Mono));
  registers.setFont(Font().setFamily(Font::Mono)).setText(" ").onActivate([&] {
    cpuRegisterEditor->loadRegisters();
    cpuRegisterEditor->setVisible();
  });

  setSize({384, 260});
  setAlignment({0.25, 0.0});
}

auto CPUDebugger::mirror(uint24 addr) -> uint24 {
  if((addr & 0x40e000) == 0x0000) addr = 0x7e0000 | (addr & 0x1fff);  //$00-3f:80-bf:0000-1fff WRAM
  return addr;
}

auto CPUDebugger::read(uint24 addr) -> uint8 {
  if((addr & 0x40e000) == 0x2000) return ~0;  //$00-3f|80-bf:2000-3fff  MMIO
  if((addr & 0x40e000) == 0x4000) return ~0;  //$00-3f|80-bf:4000-5fff  MMIO
  return SFC::bus.read(mirror(addr), SFC::cpu.r.mdr);
}

auto CPUDebugger::write(uint24 addr, uint8 data) -> void {
  if((addr & 0x40e000) == 0x2000) return;  //$00-3f|80-bf:2000-3fff  MMIO
  if((addr & 0x40e000) == 0x4000) return;  //$00-3f|80-bf:4000-5fff  MMIO
  return SFC::bus.write(mirror(addr), data);
}

auto CPUDebugger::opcodeLength(uint24 addr) -> uint {
  #define M 5
  #define X 6
  static uint lengthTable[256] = {
    2, 2, 2, 2,  2, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 3, 1, 1,  3, 3, 3, 4,
    3, 2, 4, 2,  2, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 3, 1, 1,  3, 3, 3, 4,

    1, 2, 2, 2,  3, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  3, 2, 2, 2,  1, 3, 1, 1,  4, 3, 3, 4,
    1, 2, 3, 2,  2, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 3, 1, 1,  3, 3, 3, 4,

    2, 2, 3, 2,  2, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 3, 1, 1,  3, 3, 3, 4,
    X, 2, X, 2,  2, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 3, 1, 1,  3, 3, 3, 4,

    X, 2, 2, 2,  2, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 3, 1, 1,  3, 3, 3, 4,
    X, 2, 2, 2,  2, 2, 2, 2,  1, M, 1, 1,  3, 3, 3, 4,
    2, 2, 2, 2,  3, 2, 2, 2,  1, 3, 1, 1,  3, 3, 3, 4,
  };

  uint length = lengthTable[SFC::bus.read(addr, SFC::cpu.r.mdr)];
  if(length == M) return 3 - (SFC::cpu.r.e | SFC::cpu.r.p.m);
  if(length == X) return 3 - (SFC::cpu.r.e | SFC::cpu.r.p.x);
  return length;
  #undef M
  #undef X
}

auto CPUDebugger::updateDisassembly() -> void {
  const int middle = 8;
  string line[middle * 2 + 1];

  bool e = debugger->cpuUsage.data[opcodePC] & Usage::E;
  bool m = debugger->cpuUsage.data[opcodePC] & Usage::M;
  bool x = debugger->cpuUsage.data[opcodePC] & Usage::X;
  string text = SFC::cpu.disassemble(opcodePC, e, m, x);
  line[middle] = { "> ", slice(text, 0, 7 + 22) };

  int addr = opcodePC;
  for(int o = middle - 1; o >= 0; o--) {
    for(int b = 1; b <= 4; b++) {
      if(addr - b >= 0 && (debugger->cpuUsage.data[addr - b] & Usage::Exec)) {
        addr -= b;
        e = debugger->cpuUsage.data[addr] & Usage::E;
        m = debugger->cpuUsage.data[addr] & Usage::M;
        x = debugger->cpuUsage.data[addr] & Usage::X;
        text = SFC::cpu.disassemble(addr, e, m, x);
        line[o] = { "  ", slice(text, 0, 7 + 22) };
        break;
      }
    }
  }

  addr = opcodePC;
  for(int o = middle + 1; o <= middle * 2; o++) {
    for(int b = 1; b <= 4; b++) {
      if(addr + b <= 0xffffff && (debugger->cpuUsage.data[addr + b] & Usage::Exec)) {
        addr += b;
        e = debugger->cpuUsage.data[addr] & Usage::E;
        m = debugger->cpuUsage.data[addr] & Usage::M;
        x = debugger->cpuUsage.data[addr] & Usage::X;
        text = SFC::cpu.disassemble(addr, e, m, x);
        line[o] = { "  ", slice(text, 0, 7 + 22) };
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
     "A:", hex(SFC::cpu.r.a, 4L), " X:", hex(SFC::cpu.r.x, 4L), " Y:", hex(SFC::cpu.r.y, 4L),
    " S:", hex(SFC::cpu.r.s, 4L), " D:", hex(SFC::cpu.r.d, 4L), " B:", hex(SFC::cpu.r.b, 2L), " ",
    SFC::cpu.r.p.n ? "N" : "n", SFC::cpu.r.p.v ? "V" : "v",
    SFC::cpu.r.e ? (SFC::cpu.r.p.m ? "1" : "0") : (SFC::cpu.r.p.m ? "M" : "m"),
    SFC::cpu.r.e ? (SFC::cpu.r.p.x ? "B" : "b") : (SFC::cpu.r.p.x ? "X" : "x"),
    SFC::cpu.r.p.d ? "D" : "d", SFC::cpu.r.p.i ? "I" : "i",
    SFC::cpu.r.p.z ? "Z" : "z", SFC::cpu.r.p.c ? "C" : "c",
  });
}
