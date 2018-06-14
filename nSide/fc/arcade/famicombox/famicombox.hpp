struct FamicomBox : Thread {
  shared_pointer<Emulator::Sprite> keyswitchSprite;

  enum class Exception : uint {
    Interrupt6_82Hz = 0,
    AttractionTimer = 1,
    Controller      = 2,
    KeyswitchRotate = 3,
    Coin            = 4,
    Reset           = 5,
    Watchdog        = 6,
    CATVPin1        = 7,
  };

  static auto Enter() -> void;
  auto main() -> void;

  auto load(Markup::Node node) -> bool;
  auto unload() -> void;
  auto power(bool reset) -> void;

  auto changeSlot(uint4 newSlot) -> void;
  auto trap(Exception exceptionId) -> void;
  auto pollInputs() -> void;

  auto readWRAM(uint16 addr, uint8 data) -> uint8;
  auto readIO(uint16 addr, uint8 data) -> uint8;
  auto readSRAM(uint16 addr, uint8 data) -> uint8;
  auto readCartridge(uint16 addr, uint8 data) -> uint8;
  auto writeWRAM(uint16 addr, uint8 data) -> void;
  auto writeIO(uint16 addr, uint8 data) -> void;
  auto writeSRAM(uint16 addr, uint8 data) -> void;
  auto writeCartridge(uint16 addr, uint8 data) -> void;

  auto readCHR(uint14 addr, uint8 data) -> uint8;
  auto writeCHR(uint14 addr, uint8 data) -> void;

  auto serialize(serializer& s) -> void;

  //memory map
  //0000-07ff: Game RAM
  //0800-1fff: BIOS RAM
  //2000-3fff: PPU Registers
  //4000-4015: APU/CPU Registers
  //4016-4017: APU/CPU Registers and Watchdog
  //5000-5fff: FamciomBox registers
  //6000-7fff: FamicomBox TEST Mode RAM
  //8000-ffff: cartridge ROM
  uint8 bios_prg[0x8000];
  uint8 bios_chr[0x2000];

  uint8 bios_ram[0x1800]; // actually 0x2000, but 0x800 of it is game RAM
  uint8 test_ram[0x2000];

  uint10 dip;
  uint keyswitch;

  uint8 exceptionEnable;
  uint8 exceptionTrap;

  uint4 ledSelect;
  uint3 ramProtect;
  bool  ledFlash;

  bool zapperGND;
  bool warmboot;
  bool enableControllers;
  bool swapControllers;

  uint15 attractionTimer;
  uint14 watchdog;

  uint4 cartridgeSelect;
  uint2 cartridgeRowSelect;
  bool registerLock;

  struct CoinModule {
    uint timer;
    bool min10;
    bool min20;
  } coinModule;
};

extern FamicomBox famicombox;
