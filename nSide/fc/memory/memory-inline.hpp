//Memory

auto Memory::size() const -> uint { return 0; }

//StaticRAM

StaticRAM::StaticRAM(uint size) : _size(size) { _data = new uint8[_size]; }
StaticRAM::~StaticRAM() { delete[] _data; }

auto StaticRAM::data() -> uint8* { return _data; }
auto StaticRAM::size() const -> uint { return _size; }

auto StaticRAM::read(uint16 addr, uint8) -> uint8 { return _data[addr]; }
auto StaticRAM::write(uint16 addr, uint8 data) -> void { _data[addr] = data; }
auto StaticRAM::operator[](uint16 addr) -> uint8& { return _data[addr]; }
auto StaticRAM::operator[](uint16 addr) const -> const uint8& { return _data[addr]; }

//MappedRAM

auto MappedRAM::reset() -> void {
  delete[] _data;
  _data = nullptr;
  _size = 0;
  _writeProtect = false;
}

auto MappedRAM::allocate(uint size) -> void {
  reset();
  _data = new uint8[_size = size];
  memory::fill(_data, _size, 0xff);
}

auto MappedRAM::writeProtect(bool writeProtect) -> void { _writeProtect = writeProtect; }
auto MappedRAM::data() -> uint8* { return _data; }
auto MappedRAM::size() const -> uint { return _size; }

auto MappedRAM::read(uint16 addr, uint8) -> uint8 { return _data[addr]; }
auto MappedRAM::write(uint16 addr, uint8 data) -> void { if(!_writeProtect) _data[addr] = data; }
auto MappedRAM::operator[](uint16 addr) const -> const uint8& { return _data[addr]; }

auto MappedRAM::readDirect(uint addr, uint8) -> uint8 { return _data[addr]; }
auto MappedRAM::writeDirect(uint addr, uint8 data) -> void { if(!_writeProtect) _data[addr] = data; }

//Bus

auto Bus::mirror(uint addr, uint size) -> uint {
  if(size == 0) return 0;
  uint base = 0;
  uint mask = 1 << 15;
  while(addr >= size) {
    while(!(addr & mask)) mask >>= 1;
    addr -= mask;
    if(size > mask) {
      size -= mask;
      base += mask;
    }
    mask >>= 1;
  }
  return base + addr;
}

auto Bus::reduce(uint addr, uint mask) -> uint {
  while(mask) {
    uint bits = (mask & -mask) - 1;
    addr = ((addr >> 1) & ~bits) | (addr & bits);
    mask = (mask & (mask - 1)) >> 1;
  }
  return addr;
}

//$0000-07ff = RAM (2KB)
//$0800-1fff = RAM (mirror)
//$2000-2007 = PPU
//$2008-3fff = PPU (mirror)
//$4000-4017 = APU + I/O
//$4018-ffff = Cartridge

auto Bus::read(uint16 addr, uint8 data) -> uint8 {
  if(!Model::FamicomBox()) data = cartridgeSlot[slot].readPRG(addr, data);
  data = reader[lookup[addr]](target[addr], data);
  if(cheat) {
    if(auto result = cheat.find(addr, data)) return result();
  }
  return data;
}

auto Bus::write(uint16 addr, uint8 data) -> void {
  if(!Model::FamicomBox()) cartridgeSlot[slot].writePRG(addr, data);
  return writer[lookup[addr]](target[addr], data);
}

auto Bus::readCHR(uint16 addr, uint8 data) -> uint8 {
  if(Model::FamicomBox()) return famicombox.readCHR(addr, data);
  return cartridgeSlot[slot].readCHR(addr, data);
}

auto Bus::writeCHR(uint16 addr, uint8 data) -> void {
  if(Model::FamicomBox()) return famicombox.writeCHR(addr, data);
  return cartridgeSlot[slot].writeCHR(addr, data);
}
