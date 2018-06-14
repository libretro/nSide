#include "../../icarus/core/mega-drive.cpp"

auto CartPal::megaDriveManifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "program.rom"});
  concatenate(buffer, {location, "upmem.rom"});
  return Icarus::megaDriveManifest(buffer, location);
}
