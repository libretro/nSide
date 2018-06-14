#define private protected
#include "../../icarus/core/core.hpp"
#undef private

struct CartPal : Icarus {
  //core.cpp
  CartPal();

  auto manifest(string location) -> string;
  auto import(string location) -> string;

  //atari-2600.cpp
  auto atari2600Manifest(string location) -> string;
  auto atari2600Manifest(vector<uint8_t>& buffer, string location) -> string;
  auto atari2600Import(vector<uint8_t>& buffer, string location) -> string;

  //famicom.cpp (Additional Features: NES 2.0, extra mappers, auto-selection of arcade machines)
  auto famicomManifest(string location) -> string;
  auto famicomManifest(vector<uint8_t>& buffer, string location) -> string;
  auto famicomImport(vector<uint8_t>& buffer, string location) -> string;

  //sg-1000.cpp
  auto sg1000Manifest(string location) -> string;
  auto sg1000Manifest(vector<uint8_t>& buffer, string location) -> string;
  auto sg1000Import(vector<uint8_t>& buffer, string location) -> string;

  //mega-drive.cpp (Additional Features: Lock-On)
  auto megaDriveManifest(string location) -> string;

  //vs-system.cpp
  auto vsSystemManifest(string location) -> string;
  auto vsSystemManifest(vector<uint8_t>& buffer, string location) -> string;
  auto vsSystemManifestScan(vector<Markup::Node>& roms, Markup::Node node) -> void;
  auto vsSystemImport(vector<uint8_t>& buffer, string location) -> string;

  //playchoice-10.cpp
  auto playchoice10Manifest(string location) -> string;
  auto playchoice10Manifest(vector<uint8_t>& buffer, string location) -> string;
  auto playchoice10ManifestScan(vector<Markup::Node>& roms, Markup::Node node) -> void;
  auto playchoice10Import(vector<uint8_t>& buffer, string location) -> string;

  //famicombox.cpp
  auto famicomboxManifest(string location) -> string;
  auto famicomboxManifest(vector<uint8_t>& buffer, string location) -> string;
  auto famicomboxManifestScan(vector<Markup::Node>& roms, Markup::Node node) -> void;
  auto famicomboxImport(vector<uint8_t>& buffer, string location) -> string;
};

namespace Database {
  Markup::Node Atari2600;
  Markup::Node SG1000;
  Markup::Node VSSystem;
  Markup::Node PlayChoice10;
  Markup::Node FamicomBox;
};
