namespace Heuristics {

struct MegaDrive {
  MegaDrive(vector<uint8_t>& data, string location);
  explicit operator bool() const;
  auto manifest() const -> string;

private:
  vector<uint8_t>& data;
  string location;
};

MegaDrive::MegaDrive(vector<uint8_t>& data, string location) : data(data), location(location) {
}

MegaDrive::operator bool() const {
  return data.size() >= 0x200;
}

auto MegaDrive::manifest() const -> string {
  if(!operator bool()) return {};

  static auto read16 = [&](uint addr) -> uint16_t {
    return data[addr + 0] << 8 | data[addr + 1] << 0;
  };

  static auto read24 = [&](uint addr) -> uint32_t {
    return data[addr + 0] << 16 | data[addr + 1] << 8 | data[addr + 2] << 0;
  };

  static auto read32 = [&](uint addr) -> uint32_t {
    return read16(addr + 0) << 16 | read16(addr + 2) << 0;
  };

  uint romSize = data.size();

  string ramMode = "none";

  uint32_t ramFrom = read32(0x01b4);
  uint32_t ramTo = read32(0x01b8);

  if(!(ramFrom & 1) && !(ramTo & 1)) ramMode = "hi";
  if( (ramFrom & 1) &&  (ramTo & 1)) ramMode = "lo";
  if(!(ramFrom & 1) &&  (ramTo & 1)) ramMode = "word";
  if(data[0x01b0] != 'R' || data[0x01b1] != 'A') ramMode = "none";
  if((data[0x01b2] & 0b1010'0111) != 0b1010'0000) ramMode = "none";

  uint32_t ramSize = ramTo - ramFrom + 1;
  if(ramMode == "hi") ramSize = (ramTo >> 1) - (ramFrom >> 1) + 1;
  if(ramMode == "lo") ramSize = (ramTo >> 1) - (ramFrom >> 1) + 1;
  if(ramMode == "word") ramSize = ramTo - ramFrom + 1;
  if(ramMode != "none") ramSize = bit::round(min(0x20000, ramSize));
  if(ramMode == "none") ramSize = 0;

  bool battery = data[0x01b2] & 0x40;

  bool lockon = false;
  uint32_t upmemFrom = 0;
  uint32_t upmemTo = 0;

  if(data[0x01e0] == 'R' && data[0x01e1] == 'O') {
    lockon = true;
    upmemFrom = read24(0x01e4);
    upmemTo = read24(0x01e7);
  }

  uint32_t upmemSize = lockon ? upmemTo - upmemFrom + 1 : 0;
  romSize -= upmemSize;

  string_vector regions;
  string region = slice((const char*)&data[0x1f0], 0, 16).trimRight(" ");
  if(!regions) {
    if(region == "JAPAN" ) regions.append("NTSC-J");
    if(region == "EUROPE") regions.append("PAL");
  }
  if(!regions) {
    if(region.find("J")) regions.append("NTSC-J");
    if(region.find("U")) regions.append("NTSC-U");
    if(region.find("E")) regions.append("PAL");
    if(region.find("W")) regions.append("NTSC-J", "NTSC-U", "PAL");
  }
  if(!regions && region.size() == 1) {
    uint8_t field = region.hex();
    if(field & 0x01) regions.append("NTSC-J");
    if(field & 0x04) regions.append("NTSC-U");
    if(field & 0x08) regions.append("PAL");
  }
  if(!regions) {
    regions.append("NTSC-J");
  }

  string output;
  output.append("game\n");
  output.append("  sha256: ", Hash::SHA256(data).digest(), "\n");
  output.append("  label:  ", Location::prefix(location), "\n");
  output.append("  name:   ", Location::prefix(location), "\n");
  output.append("  region: ", regions.left(), "\n");
  output.append("  board\n");
  if(lockon) output.append("    lock-on\n");
  output.append(Memory{}.type("ROM").size(romSize).content("Program").text());
  if(upmemSize) output.append(Memory{}.type("ROM").size(upmemSize).content("UPMEM").text());
  if(ramSize && ramMode != "none") {
    output.append(battery
    ? Memory{}.type("RAM").size(ramSize).content("Save").text()
    : Memory{}.type("RAM").size(ramSize).content("Save").isVolatile().text());
    output.append("      mode: ", ramMode, "\n");
    output.append("      offset: 0x", hex(ramFrom), "\n");
  }
  return output;
}

}
