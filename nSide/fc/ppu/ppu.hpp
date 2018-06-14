struct PPU : Thread, PPUcounter {
  enum class Version : uint {
    //YIQ
    RP2C02C,
    RP2C02E,
    RP2C02G,

    //RGB
    RP2C03B,
    RP2C03G,
    RP2C04_0001,
    RP2C04_0002,
    RP2C04_0003,
    RP2C04_0004,
    RC2C03B,
    RC2C03C,
    RC2C05_01,
    RC2C05_02,
    RC2C05_03,
    RC2C05_04,
    RC2C05_05,

    //YUV
    RP2C07,
    UA6538,
  } version;

  alwaysinline auto ntsc() const -> bool { return version >= Version::RP2C02C && version <= Version::RP2C02G; }
  alwaysinline auto pal()  const -> bool { return version >= Version::RP2C07  && version <= Version::UA6538;  }
  alwaysinline auto rgb()  const -> bool { return version >= Version::RP2C03B && version <= Version::RC2C05_05; }

  inline auto rate() const -> uint { return Region::PAL() || Region::Dendy() ? 5 : 4; }
  inline auto vlines() const -> uint { return Region::PAL() || Region::Dendy() ? 312 : 262; }

  PPU(bool side);
  ~PPU();

  alwaysinline auto step(uint clocks) -> void;

  static auto Enter() -> void;
  auto main() -> void;
  auto load(Markup::Node) -> bool;
  auto power(bool reset) -> void;

  auto extIn() -> uint4;
  auto extOut() -> uint4;

  //io.cpp
  auto readCIRAM(uint12 addr) -> uint8;
  auto writeCIRAM(uint12 addr, uint8 data) -> void;

  alwaysinline auto readCGRAM(uint5 addr) -> uint8;
  alwaysinline auto writeCGRAM(uint5 addr, uint8 data) -> void;

  auto readIO(uint16 addr, uint8 data) -> uint8;
  auto writeIO(uint16 addr, uint8 data) -> void;

  //render.cpp
  auto enable() const -> bool;
  auto nametableAddress() const -> uint;
  auto scrollX() const -> uint;
  auto scrollY() const -> uint;

  auto loadCHR(uint14 addr) -> uint8;

  auto scrollX_increment() -> void;
  auto scrollY_increment() -> void;

  auto renderPixel() -> void;
  auto renderSprite() -> void;
  auto renderScanline() -> void;

  //serialization.cpp
  auto serialize(serializer&) -> void;

  uint8 ciram[4096];  //2048 in Famicom and PlayChoice-10
  uint8 cgram[32];
  uint8 oam[256];

  const bool side;  //0: main, 1: sub (VS. System only)

  static const uint9 RP2C03[16 * 4];
  static const uint9 RP2C04_0001[16 * 4];
  static const uint9 RP2C04_0002[16 * 4];
  static const uint9 RP2C04_0003[16 * 4];
  static const uint9 RP2C04_0004[16 * 4];

  struct IO {
    uint14 chrAddressBus;
    uint8 mdr;
    uint mdrDecay[8];

    uint8 busData;

    union {
      uint value;
      NaturalBitField<uint, 0, 4> tileX;
      NaturalBitField<uint, 5, 9> tileY;
      NaturalBitField<uint,10,11> nametable;
      NaturalBitField<uint,10,10> nametableX;
      NaturalBitField<uint,11,11> nametableY;
      NaturalBitField<uint,12,14> fineY;
      NaturalBitField<uint, 0,14> address;
      NaturalBitField<uint, 0, 7> addressLo;
      NaturalBitField<uint, 8,14> addressHi;
      NaturalBitField<uint,15,15> latch;
      NaturalBitField<uint,16,18> fineX;
    } v, t;

    bool nmiHold;
    bool nmiFlag;

    //$2000  PPUCTRL
    bool nmiEnable;
    bool masterSelect;
    uint spriteHeight;
    uint bgAddress;
    uint objAddress;
    uint vramIncrement;

    //$2001  PPUMASK
    uint3 emphasis;
    bool objEnable;
    bool bgEnable;
    bool objEdgeEnable;
    bool bgEdgeEnable;
    bool grayscale;

    //$2002  PPUSTATUS
    bool spriteZeroHit;
    bool spriteOverflow;

    //$2003  OAMADDR
    uint8 oamAddress;
  } io;

  struct OAM {
    //serialization.cpp
    auto serialize(serializer&) -> void;

    uint8 id = 64;
    uint8 y = 0xff;
    uint8 tile = 0xff;
    uint8 attr = 0xff;
    uint8 x = 0xff;

    uint8 tiledataLo = 0;
    uint8 tiledataHi = 0;
  };

  struct Latches {
    uint16 nametable;
    uint16 attribute;
    uint16 tiledataLo;
    uint16 tiledataHi;

    uint oamIterator;
    uint oamCounter;

    OAM oam[8];   //primary
    OAM soam[8];  //secondary
  } l;

private:
//Emulator::Raster raster;
  uint32* output = nullptr;

  uint4 _extOut;

  auto scanline() -> void;
  auto frame() -> void;
  auto refresh() -> void;

  friend class Zapper;
  friend class BeamGun;
  friend class System;
};

extern PPU ppuM;
extern PPU ppuS;
