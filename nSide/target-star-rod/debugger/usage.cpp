Usage::Usage() {
  data = nullptr;
  size = 0;
}

Usage::~Usage() {
  if(data) delete[] data;
}

auto Usage::allocate(uint size_) -> void {
  if(data) delete[] data;
  size = size_;
  data = new uint8_t[size]();
}

auto Debugger::loadUsage() -> void {
  file fp;

  //if cartridge image was modified after the usage files,
  //then it is possible that the memory map has changed.
  //will print invalidation message when files do not exist as well.

  if(file::timestamp({program->mediumPaths(1), "program.rom"    }, file::time::modify) >=
     file::timestamp({program->mediumPaths(1), "debug/usage.cpu"}, file::time::modify)
  ) {
    print("CPU usage invalidated\n");
  } else {
    if(fp.open({program->mediumPaths(1), "debug/usage.cpu"}, file::mode::read)) {
      fp.read(cpuUsage.data, min(cpuUsage.size, fp.size()));
      fp.close();
    }
  }

  if(file::timestamp({program->mediumPaths(1), "program.rom"    }, file::time::modify) >=
     file::timestamp({program->mediumPaths(1), "debug/usage.apu"}, file::time::modify)
  ) {
    print("APU usage invalidated\n");
  } else {
    if(fp.open({program->mediumPaths(1), "debug/usage.apu"}, file::mode::read)) {
      fp.read(apuUsage.data, min(apuUsage.size, fp.size()));
      fp.close();
    }
  }
}

auto Debugger::saveUsage() -> void {
  directory::create({program->mediumPaths(1), "debug/"});

  string filename;
  filename = {program->mediumPaths(1), "debug/usage.cpu"};
  file::write(filename, cpuUsage.data, cpuUsage.size);
  filename = {program->mediumPaths(1), "debug/usage.apu"};
  file::write(filename, apuUsage.data, apuUsage.size);
}

auto Debugger::resetUsage() -> void {
  if(cpuUsage.data) memory::fill(cpuUsage.data, cpuUsage.size, 0x00);
  if(apuUsage.data) memory::fill(apuUsage.data, apuUsage.size, 0x00);
}
