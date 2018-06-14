struct Window {
  auto power() -> void;

  alwaysinline auto buildTable(uint bg_id, bool screen) -> void;
  auto buildTables(uint bg_id) -> void;

  auto serialize(serializer&) -> void;

  struct ID { enum : uint { COL = 5 }; };

  struct IO {
    struct Layer {
      bool oneEnable;
      bool oneInvert;
      bool twoEnable;
      bool twoInvert;
      uint2 mask;
      bool aboveEnable;
      bool belowEnable;
    } bg1, bg2, bg3, bg4, obj;

    struct Color {
      bool oneEnable;
      bool oneInvert;
      bool twoEnable;
      bool twoInvert;
      uint2 mask;
      uint2 aboveMask;
      uint2 belowMask;
    } col;

    uint8 oneLeft;
    uint8 oneRight;
    uint8 twoLeft;
    uint8 twoRight;
  } io;

  struct Cache {
    uint8 above[256], below[256];
  } cache[6];

  friend class PPU;
};
