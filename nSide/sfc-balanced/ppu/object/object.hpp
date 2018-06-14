struct OAM {
  auto read(uint10 addr) -> uint8;
  auto write(uint10 addr, uint8 data) -> void;

  struct Object {
    uint8 width;
    uint8 height;

    uint9 x;
    uint8 y;
    uint8 character;
    bool  nameselect;
    bool  vflip;
    bool  hflip;
    uint2 priority;
    uint3 palette;
    bool  size;

    auto updateSize() -> void;
  } object[128];
};

struct Object {
  alwaysinline auto addressReset() -> void;
  alwaysinline auto setFirstSprite() -> void;
  auto power() -> void;
  alwaysinline auto onScanline(uint id) -> bool;
  alwaysinline auto loadTiles(uint id) -> void;
  alwaysinline auto renderTile(uint n) -> void;
  auto render() -> void;
  auto renderRTO() -> void;

  auto serialize(serializer&) -> void;

  struct ID { enum : uint { OBJ = 4 }; };

  const uint8 PRIORITY_NONE = ~1;

  OAM oam;

  struct IO {
    bool aboveEnable;
    bool belowEnable;
    bool interlace;

    uint3  baseSize;
    uint2  nameselect;
    uint16 tiledataAddress;
    uint7  firstSprite;

    uint priority[4];

    bool timeOver;
    bool rangeOver;
  } io;

  uint8 itemList[32];
  struct Tile {
    uint16 x;
    uint16 y;
    uint2  priority;
    uint8  palette;
    bool   hflip;
    uint16 tile;
  } tileList[34];

  struct State {
    //uint x;
    //uint y;

    uint itemCount;
    uint tileCount;

    //bool active;
    //Item item[2][32];
    //Tile tile[2][34];
  } t;

  struct Output {
    uint8 palette[256];
    uint8 priority[256];
  } output;

  struct Cache {
    uint8  baseSize;
    uint8  nameselect;
    uint16 tiledataAddress;
  } cache;

  friend class PPU;
};
