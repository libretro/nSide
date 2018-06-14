#include "../laevateinn.hpp"
unique_pointer<Debugger> debugger;

#include "hook.cpp"
#include "usage.cpp"

Debugger::Debugger() {
  debugger = this;
  paused = true;

  flags.step = false;
  flags.cpu.stepInto = false;
  flags.cpu.nmi = false;
  flags.cpu.irq = false;
  flags.smp.stepInto = false;

  debug.cpu = true;
  debug.smp = false;

  cpuUsage.allocate(16 * 1024 * 1024);
  apuUsage.allocate(64 * 1024);

  SFC::debugger.cpu.execute = {&Debugger::cpu_execute, this};
  SFC::debugger.cpu.read = {&Debugger::cpu_read, this};
  SFC::debugger.cpu.write = {&Debugger::cpu_write, this};

  SFC::debugger.cpu.nmi = {&Debugger::cpu_nmi, this};
  SFC::debugger.cpu.irq = {&Debugger::cpu_irq, this};

  SFC::debugger.smp.execute = {&Debugger::smp_execute, this};
  SFC::debugger.smp.read = {&Debugger::smp_read, this};
  SFC::debugger.smp.write = {&Debugger::smp_write, this};

  SFC::debugger.ppu.vram.read = {&Debugger::ppu_vramRead, this};
  SFC::debugger.ppu.vram.write = {&Debugger::ppu_vramWrite, this};

  SFC::debugger.ppu.oam.read = {&Debugger::ppu_oamRead, this};
  SFC::debugger.ppu.oam.write = {&Debugger::ppu_oamWrite, this};

  SFC::debugger.ppu.cgram.read = {&Debugger::ppu_cgramRead, this};
  SFC::debugger.ppu.cgram.write = {&Debugger::ppu_cgramWrite, this};
}

auto Debugger::run() -> void {
  if(paused) {
    usleep(2000);
    return;
  }

  emulator->run();
  if(cpuDebugger->autoUpdate.checked()) cpuDebugger->updateDisassembly();
  if(smpDebugger->autoUpdate.checked()) smpDebugger->updateDisassembly();
  if(memoryEditor->autoUpdate.checked()) memoryEditor->updateView();
  if(propertiesViewer->autoUpdate.checked()) propertiesViewer->updateProperties();
  if(tileViewer->autoUpdate.checked()) tileViewer->updateTiles();
  if(bgViewer->autoUpdate.checked()) bgViewer->updateTiles();
  if(paletteViewer->autoUpdate.checked()) paletteViewer->updateColors();
}

auto Debugger::echo(const string& text) -> void {
  consoleWindow->print(text);
}

auto Debugger::resume() -> void {
  if(!paused) return;
  paused = false;
  consoleWindow->runButton.setText("Stop");
}

auto Debugger::suspend() -> void {
  if(paused) return;
  paused = true;
  flags.step = false;
  flags.cpu.stepInto = false;
  flags.cpu.nmi = false;
  flags.cpu.irq = false;
  flags.smp.stepInto = false;
  consoleWindow->runButton.setText("Run");
}
