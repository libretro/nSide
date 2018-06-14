#include <fc/fc.hpp>

namespace Famicom {

Bus busM;
Bus busS;

Bus::~Bus() {
  if(lookup) delete[] lookup;
  if(target) delete[] target;
}

auto Bus::reset() -> void {
  for(auto id : range(256)) {
    reader[id].reset();
    writer[id].reset();
    counter[id] = 0;
  }

  if(lookup) delete[] lookup;
  if(target) delete[] target;

  lookup = new uint8 [16 * 1024 * 1024]();
  target = new uint32[16 * 1024 * 1024]();

  reader[0] = [](uint16, uint8 data) -> uint8 { return data; };
  writer[0] = [](uint16, uint8) -> void {};
}

auto Bus::map(
  const function<uint8 (uint16, uint8)>& read,
  const function<void (uint16, uint8)>& write,
  const string& addr, uint size, uint base, uint mask
) -> void {
  uint id = 1;
  while(counter[id]) {
    if(++id >= 256) return print("FC error: bus map exhausted\n");
  }

  reader[id] = read;
  writer[id] = write;

  auto addrs = addr.split(",");
  for(auto& addr : addrs) {
    auto addrRange = addr.split("-", 1L);
    uint addrLo = addrRange(0).hex();
    uint addrHi = addrRange(1, addrRange(0)).hex();

    for(uint addr = addrLo; addr <= addrHi; addr++) {
      uint pid = lookup[addr];
      if(pid && --counter[pid] == 0) {
        reader[pid].reset();
        writer[pid].reset();
      }

      uint offset = reduce(addr, mask);
      if(size) offset = base + mirror(offset, size - base);
      lookup[addr] = id;
      target[addr] = offset;
      counter[id]++;
    }
  }
}

auto Bus::unmap(const string& addr) -> void {
  auto addrs = addr.split(",");
  for(auto& addr : addrs) {
    auto addrRange = addr.split("-", 1L);
    uint addrLo = addrRange(0).hex();
    uint addrHi = addrRange(1, addrRange(1)).hex();

    for(uint addr = addrLo; addr <= addrHi; addr++) {
      uint pid = lookup[addr];
      if(pid && --counter[pid] == 0) {
        reader[pid].reset();
        writer[pid].reset();
      }

      lookup[addr] = 0;
      target[addr] = 0;
    }
  }
}

//$0000-07ff = RAM (2KB)
//$0800-1fff = RAM (mirror)
//$2000-2007 = PPU
//$2008-3fff = PPU (mirror)
//$4000-4017 = APU + I/O
//$4018-ffff = Cartridge

}
