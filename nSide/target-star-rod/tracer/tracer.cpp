#include "../laevateinn.hpp"
unique_pointer<Tracer> tracer;

Tracer::Tracer() {
  tracer = this;
  mask = false;
}

auto Tracer::resetMask() -> void {
  memory::fill(cpuMask, 0x200000, 0);
  memory::fill(smpMask, 0x2000, 0);
}

auto Tracer::maskCPU(uint24 addr) -> bool {
  if(mask == false) return false;
  if(cpuMask[addr >> 3] & (1 << (addr & 7))) return true;
  cpuMask[addr >> 3] |= 1 << (addr & 7);
  return false;
}

auto Tracer::maskSMP(uint16 addr) -> bool {
  if(mask == false) return false;
  if(smpMask[addr >> 3] & (1 << (addr & 7))) return true;
  smpMask[addr >> 3] |= 1 << (addr & 7);
  return false;
}

auto Tracer::enabled() -> bool {
  return fp.open();
}

auto Tracer::enable(bool state) -> void {
  if(state == false) {
    debugger->print("Tracer disabled\n");
    fp.close();
    return;
  }

  directory::create({program->mediumPaths(1), "debug/"});

  //try not to overwrite existing traces: scan from 001-999.
  //if all files exist, use 000, even if it overwrites another log.
  uint n = 1;
  do {
    if(!file::exists({program->mediumPaths(1), "debug/trace-", pad(n, 3L, '0'), ".log"})) break;
  } while(++n <= 999);

  string filename = {program->mediumPaths(1), "debug/trace-", pad(n, 3L, '0'), ".log"};
  if(fp.open(filename, file::mode::write) == false) return;
  debugger->print("Tracing to ", filename, "\n");
}
