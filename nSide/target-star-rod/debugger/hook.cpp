//S-CPU
//=====

auto Debugger::cpu_execute(uint24 addr) -> void {
  cpuUsage.data[addr] |= Usage::Exec;
  if(SFC::cpu.r.e) cpuUsage.data[addr] |= Usage::E;
  if(SFC::cpu.r.p.m) cpuUsage.data[addr] |= Usage::M;
  if(SFC::cpu.r.p.x) cpuUsage.data[addr] |= Usage::X;
  cpuDebugger->opcodePC = addr;
  bool breakpointHit = breakpointEditor->testExecCPU(addr);

  if((debug.cpu && tracer->enabled() && !tracer->maskCPU(addr))
  || (debug.cpu && flags.step)
  || flags.cpu.stepInto
  || breakpointHit
  ) {
    string text = SFC::cpu.disassemble(addr, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x);

    if(debug.cpu && tracer->enabled()) tracer->print(text, "\n");
    if((debug.cpu && flags.step) || flags.cpu.stepInto || breakpointHit) {
      print(text, "\n");
      if(debug.cpu && flags.step) {
        consoleWindow->stepButton.setFocused();
      }
      if(flags.cpu.stepInto) {
        cpuDebugger->stepInto.setFocused();
        cpuDebugger->updateDisassembly();
      }
      suspend();
      SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
    }
  }
}

auto Debugger::cpu_read(uint24 addr, uint8 data) -> void {
  cpuUsage.data[addr] |= Usage::Read;
  bool breakpointHit = breakpointEditor->testReadCPU(addr);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::cpu_write(uint24 addr, uint8 data) -> void {
  cpuUsage.data[addr] |= Usage::Write;
  bool breakpointHit = breakpointEditor->testWriteCPU(addr, data);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::cpu_nmi() -> void {
  if(flags.cpu.nmi) {
    print("CPU NMI\n");  //, text, "\n");
    flags.cpu.stepInto = true;
  }
}

auto Debugger::cpu_irq() -> void {
  if(flags.cpu.irq) {
    print("CPU IRQ\n");
    flags.cpu.stepInto = true;
  }
}

//S-SMP
//=====

auto Debugger::smp_execute(uint16 addr) -> void {
  apuUsage.data[addr] |= Usage::Exec;
  if(SFC::smp.r.p.p) apuUsage.data[addr] |= Usage::P;
  smpDebugger->opcodePC = addr;
  bool breakpointHit = breakpointEditor->testExecSMP(addr);

  if((debug.cpu && tracer->enabled() && !tracer->maskSMP(addr))
  || (debug.smp && flags.step)
  || flags.smp.stepInto
  || breakpointHit
  ) {
    string text = SFC::smp.disassemble(addr, SFC::smp.r.p.p);

    if(debug.smp && tracer->enabled()) tracer->print(text, "\n");
    if((debug.smp && flags.step) || flags.smp.stepInto || breakpointHit) {
      print(text, "\n");
      if(debug.smp && flags.step) {
        consoleWindow->stepButton.setFocused();
      }
      if(flags.smp.stepInto) {
        smpDebugger->stepInto.setFocused();
        smpDebugger->updateDisassembly();
      }
      suspend();
      SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
    }
  }
}

auto Debugger::smp_read(uint16 addr, uint8 data) -> void {
  apuUsage.data[addr] |= Usage::Read;
  bool breakpointHit = breakpointEditor->testReadSMP(addr);

  if(breakpointHit) {
    print(SFC::smp.disassemble(smpDebugger->opcodePC, SFC::smp.r.p.p), "\n");
    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::smp_write(uint16 addr, uint8 data) -> void {
  apuUsage.data[addr] |= Usage::Write;
  bool breakpointHit = breakpointEditor->testWriteSMP(addr, data);

  if(breakpointHit) {
    print(SFC::smp.disassemble(smpDebugger->opcodePC, SFC::smp.r.p.p), "\n");
    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

//S-PPU
//=====

auto Debugger::ppu_vramRead(uint17 addr, uint8 data) -> void {
  bool breakpointHit = breakpointEditor->testReadVRAM(addr);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::ppu_vramWrite(uint17 addr, uint8 data) -> void {
  bool breakpointHit = breakpointEditor->testWriteVRAM(addr, data);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::ppu_oamRead(uint10 addr, uint8 data) -> void {
  bool breakpointHit = breakpointEditor->testReadOAM(addr);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::ppu_oamWrite(uint10 addr, uint8 data) -> void {
  bool breakpointHit = breakpointEditor->testWriteOAM(addr, data);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::ppu_cgramRead(uint9 addr, uint8 data) -> void {
  bool breakpointHit = breakpointEditor->testReadCGRAM(addr);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}

auto Debugger::ppu_cgramWrite(uint9 addr, uint8 data) -> void {
  bool breakpointHit = breakpointEditor->testWriteCGRAM(addr, data);

  if(breakpointHit) {
    print(SFC::cpu.disassemble(cpuDebugger->opcodePC, SFC::cpu.r.e, SFC::cpu.r.p.m, SFC::cpu.r.p.x), "\n");

    suspend();
    SFC::scheduler.exit(SFC::Scheduler::Event::Debugger);
  }
}
