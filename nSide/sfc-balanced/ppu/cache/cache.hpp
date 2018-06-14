struct Cache {
  Cache();
  ~Cache();

  auto tile2bpp(uint16 tile) -> uint8*;
  auto tile4bpp(uint16 tile) -> uint8*;
  auto tile8bpp(uint16 tile) -> uint8*;
  auto tile(uint bpp, uint16 tile) -> uint8*;
  auto flush() -> void;

  uint8* tiledata[3];
  uint8* tilevalid[3];
};
