struct Scene {
  virtual auto reset() -> void = 0;
  virtual auto run() -> void = 0;

  virtual auto loadSprites(vector<Sprite*>& sprites) -> void = 0;
  virtual auto updateSprites() -> void = 0;
};
