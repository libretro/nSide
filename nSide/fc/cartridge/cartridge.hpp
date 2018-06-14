#include "chip/chip.hpp"
#include "board/board.hpp"

struct Cartridge : Thread {
  Cartridge(uint slot);

  static void Enter();
  void main();

  auto pathID() const -> uint { return information.pathID; }
  auto region() const -> string { return information.region; }
  auto sha256() const -> string { return information.sha256; }
  auto manifest() const -> string;
  auto title() const -> string;

  auto load() -> bool;
  auto save() -> void;
  auto unload() -> void;

  auto power(bool reset) -> void;

  auto serialize(serializer&) -> void;

  struct Information {
    uint pathID = 0;
    string region;
    string sha256;
  } information;

//privileged:
  shared_pointer<Board> board;
  const uint slot;

  auto readPRG(uint addr, uint8 data) -> uint8;
  auto writePRG(uint addr, uint8 data) -> void;

  auto readCHR(uint addr, uint8 data) -> uint8;
  auto writeCHR(uint addr, uint8 data) -> void;

  //scanline() is for debugging purposes only:
  //boards must detect scanline edges on their own
  auto scanline(uint y) -> void;

private:
  Emulator::Game game;
  Emulator::Game slotFamicomDiskSystem;
  Markup::Node boardNode;

  //load.cpp
  auto loadBoard(string) -> Markup::Node;
  auto loadCartridge() -> void;
  auto setupVS(Markup::Node&, uint) -> void;

  auto loadMemory(MappedRAM&, Emulator::Game::Memory, bool required, maybe<uint> id = nothing) -> void;
  auto loadMemory(MappedRAM&, Markup::Node, bool required, maybe<uint> id = nothing) -> void;

  //save.cpp
  auto saveCartridge() -> void;

  auto saveMemory(MappedRAM&, Emulator::Game::Memory, maybe<uint> = nothing) -> void;
  auto saveMemory(MappedRAM&, Markup::Node, maybe<uint> = nothing) -> void;

  friend class Interface;
  friend class System;
};

extern vector<Cartridge> cartridgeSlot;
