#include "../../icarus/core/core.cpp"

CartPal::CartPal() {
  Icarus();
  Database::Atari2600 = BML::unserialize(string::read(locate("Database/Atari 2600.bml")));
  Database::SG1000 = BML::unserialize(string::read(locate("Database/SG-1000.bml")));
  Database::VSSystem = BML::unserialize(string::read(locate("Database/VS. System.bml")));
  Database::PlayChoice10 = BML::unserialize(string::read(locate("Database/PlayChoice-10.bml")));
  Database::FamicomBox = BML::unserialize(string::read(locate("Database/FamicomBox.bml")));
}

auto CartPal::manifest(string location) -> string {
  location.transform("\\", "/").trimRight("/").append("/");
  if(!directory_exists(location)) return {};  //change by the libretro team

  auto type = Location::suffix(location).downcase();
  if(type == ".a26") return atari2600Manifest(location);
  if(type == ".fc") return famicomManifest(location);
  if(type == ".sg1000") return sg1000Manifest(location);
  if(type == ".md") return megaDriveManifest(location);
  if(type == ".vs") return vsSystemManifest(location);
  if(type == ".pc10") return playchoice10Manifest(location);
  if(type == ".fcb") return famicomboxManifest(location);

  return Icarus::manifest(location);
}

auto CartPal::import(string location) -> string {
  errorMessage = {};
  missingFiles = {};

  location.transform("\\", "/").trimRight("/");
  if(!exists(location)) return failure("file does not exist");  //change by the libretro team
  if(!readable(location)) return failure("file is unreadable");  //change by the libretro team

  auto name = Location::prefix(location);
  auto type = Location::suffix(location).downcase();
  if(!name || !type) return failure("invalid file name");

  auto buffer = read(location);  //change by the libretro team
  if(!buffer) return failure("file is empty");

  if(type == ".zip") {
    Decode::ZIP zip;
    if(!zip.open(location)) return failure("ZIP archive is invalid");
    if(!zip.file) return failure("ZIP archive is empty");

    name = Location::prefix(zip.file[0].name);
    type = Location::suffix(zip.file[0].name).downcase();
    buffer = zip.extract(zip.file[0]);
  }

  if(type == ".a26") return atari2600Import(buffer, location);
  if(type == ".fc" || type == ".nes") return famicomImport(buffer, location);
  if(type == ".sg1000") return sg1000Import(buffer, location);
  if(type == ".vs") return vsSystemImport(buffer, location);
  if(type == ".pc10") return playchoice10Import(buffer, location);
  if(type == ".fcb") return famicomboxImport(buffer, location);

  return Icarus::import(location);
}
