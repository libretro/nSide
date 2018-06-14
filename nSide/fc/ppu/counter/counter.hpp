struct PPUcounter {
  alwaysinline auto tick(uint clocks) -> void;

  alwaysinline auto field() const -> bool;
  alwaysinline auto vcounter() const -> uint16;
  alwaysinline auto hcounter() const -> uint16;
  alwaysinline auto skip() const -> bool;
  inline auto lineclocks() const -> uint16;

  inline auto reset() -> void;
  function<auto () -> void> scanline;
  auto serialize(serializer&) -> void;

//private:
  inline auto vcounterTick() -> void;

  struct {
    bool field;
    uint16 vcounter;
    uint16 hcounter;
    bool skip;
  } status;
};
