//MOS 6507

struct CPU : Processor::MOS6502, Thread {
  CPU();

  static auto Enter() -> void;
  auto main() -> void;
  auto load(Markup::Node) -> bool;
  auto power() -> void;
  auto reset() -> void;

  //memory.cpp
  auto read(uint16 addr) -> uint8 override;
  auto write(uint16 addr, uint8 data) -> void override;

  //timing.cpp
  auto step(uint clocks) -> void;
  auto lastCycle() -> void;

  auto rdyLine(bool) -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  auto nmi(uint16&) -> void {}

private:
  struct IO {
    bool rdyLine;
  } io;
};

extern CPU cpu;
