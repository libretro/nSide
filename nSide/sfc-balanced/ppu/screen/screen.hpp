struct Screen {
  auto power() -> void;

  auto blend(uint x, uint y) const -> uint16;

  auto scanline() -> void;
  alwaysinline auto renderBlack() -> void;
  alwaysinline auto getPixelAbove(uint x) -> uint16;
  alwaysinline auto getPixelBelow(uint x) -> uint16;
  alwaysinline auto render() -> void;

  auto serialize(serializer&) -> void;

  struct ID { enum : uint { BACK = 5 }; };

  uint16 cgram[256];

  struct IO {
    bool blendMode;
    bool directColor;

    bool colorMode;
    bool colorHalve;
    struct Layer {
      bool colorEnable;
    } bg1, bg2, bg3, bg4, obj, back;

    uint16 color;
  } io;

  struct Math {
    //struct Screen {
    //  uint16 color;
    //  bool colorEnable;
    //} above, below;
    //bool transparent;
    //bool blendMode;
    bool colorHalve;
  } math;

  struct Output {
    alwaysinline auto plotAbove(uint x, uint16 color, uint priority, uint source, bool exemption) -> void;
    alwaysinline auto plotBelow(uint x, uint16 color, uint priority, uint source, bool exemption) -> void;

    struct Pixel {
      //bgr555 color data for main/subscreen pixels: 0x0000 = transparent / use palette color # 0
      //needs to be bgr555 instead of palette index for direct color mode ($2130 bit 0) to work
      uint16 color;
      //priority level of src_n. to set src_n,
      //the priority of the pixel must be >pri_n
      uint priority;
      //indicates source of palette # for main/subscreen (BG1-4, OBJ, or back)
      uint source;
      //color_exemption -- true when bg == OBJ && palette index >= 192, disables color blend effects
      bool exemption;
    } above[256], below[256];
  } output;

  friend class PPU;
};
