Sprite::Sprite() {
  img = nullptr;

  x = 0;
  y = 0;
  ox = 0;
  oy = 0;
}

Sprite::Sprite(any x, any y, any ox, any oy, image& img) {
  set(x, y, ox, oy, img);
}

auto Sprite::set(any _x, any _y, any _ox, any _oy) -> void {
  if(_x.is<int>())    x = _x.get<int>();
  if(_x.is<uint>())   x = _x.get<uint>();
  if(_x.is<double>()) x = _x.get<double>() * graphics->buffer.width();

  if(_y.is<int>())    y = _y.get<int>();
  if(_y.is<uint>())   y = _y.get<uint>();
  if(_y.is<double>()) y = _y.get<double>() * graphics->buffer.height();

  if(_ox.is<int>())    ox = _ox.get<int>();
  if(_ox.is<uint>())   ox = _ox.get<uint>();
  if(_ox.is<double>()) ox = _ox.get<double>() * img->width();

  if(_oy.is<int>())    oy = _oy.get<int>();
  if(_oy.is<uint>())   oy = _oy.get<uint>();
  if(_oy.is<double>()) oy = _oy.get<double>() * img->height();
}

auto Sprite::set(any x, any y, any ox, any oy, image& _img) -> void {
  img = &_img;
  set(x, y, ox, oy);
}

auto Sprite::width() -> uint {
  return img ? img->width() : 0;
}

auto Sprite::height() -> uint {
  return img ? img->height() : 0;
}
