struct Sprite {
  Sprite();
  Sprite(any x, any y, any ox, any oy, image&);

  auto set(any, any, any, any) -> void;
  auto set(any, any, any, any, image&) -> void;

  auto width() -> uint;
  auto height() -> uint;

  int x;
  int y;
  int ox;
  int oy;
  image* img;
};

struct Graphics {
  Graphics();

  auto initialize() -> void;
  auto reset() -> void;
  auto run() -> void;

  auto drawTextCenter(string_view text, image& img) -> void;

  image buffer;
  double scale;
  uint step;

  vector<Sprite*> sprites;
};

extern unique_pointer<Graphics> graphics;
