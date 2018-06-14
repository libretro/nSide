#include "../shimai.hpp"
#include "font.cpp"
unique_pointer<Theme> theme;

Theme::Theme() {
  theme = this;
}

auto Theme::load(string themeName) -> void {
  name = themeName;

  string path = {"Themes/", name, "/"};
  auto manifest = BML::unserialize(file::read(locate({path, "manifest.bml"})));

  backgroundColor = 0xff000000 | manifest["theme/background-color"].natural();
  font.color      = 0xff000000 | manifest["theme/font-color"].natural();

  double scale = graphics->scale;
  uint x,w,h;

  image texture;
  if(file::exists(locate({path, "graphics.png"}))) texture.load(locate({path, "graphics.png"}));

  static auto extract = [&](string name, image& img, bool doScale) -> image {
    auto spriteNode = manifest[{"theme/sprites/", name}];
    if(texture && spriteNode) {
      uint x = spriteNode["x"].natural();
      uint y = spriteNode["y"].natural();
      uint w = spriteNode["w"].natural();
      uint h = spriteNode["h"].natural();
      img.allocate(w, h);
      img.impose(image::blend::sourceColor, 0, 0, texture, x, y, w, h);
    } else {
      img.load(locate({path, name, ".png"}));
    }
    if(doScale) img.scale(img.width() * scale, img.height() * scale, false);
    return img;
  };

  extract("menubarU",        menubarU,           true);
  extract("menubarL",        menubarL,           true);
  extract("captionTitle",    captionTitle,       true);
  extract("gameCard",        gameCardBase,       true);
  extract("gameCardActive",  gameCardActiveBase, true);
  extract("settingsDisplay", settingsDisplay,    true);
  extract("settingsOption",  settingsOption,     true);

//image settingsBack;
//extract("settingsBack", settingsBack, false);
//w = 2 * 32 * scale;
//h = settingsBack.height() * scale;
//settingsBack.scale(w, h, false);
//x = (menubarU.width() - w) / 2;
//menubarU.impose(image::blend::sourceAlpha, x, 0, settingsBack, 0, 0, w, h);

  image cursorPart;

  gameCursor.allocate(gameCardBase.width() + 8 * scale, gameCardBase.height() + 8 * scale);
  gameCursor.fill(0x00000000);
  static auto imposeGameCursorPart = [&](string id, uint x, uint y) -> void {
    extract({"cursorGame", id}, cursorPart, false);
    cursorPart.crop(1, 1, 30, 30);
    cursorPart.scale(w, h, false);
    gameCursor.impose(image::blend::sourceAlpha, x, y, cursorPart, 0, 0, w, h);
  };
  w = 10 * scale;
  h = 10 * scale;
  imposeGameCursorPart("1", 0,                      0);
  imposeGameCursorPart("3", gameCursor.width() - w, 0);
  imposeGameCursorPart("7", 0,                      gameCursor.height() - h);
  imposeGameCursorPart("9", gameCursor.width() - w, gameCursor.height() - h);
  w = gameCursor.width() - 16 * scale;
  h = 10 * scale;
  imposeGameCursorPart("2", 8 * scale, 0);
  imposeGameCursorPart("8", 8 * scale, gameCursor.height() - h);
  w = 10 * scale;
  h = gameCursor.height() - 16 * scale;
  imposeGameCursorPart("4", 0,                      8 * scale);
  imposeGameCursorPart("6", gameCursor.width() - w, 8 * scale);
  gameCursor.crop(scale * 2, scale * 2, gameCursor.width() - scale * 2, gameCursor.height() - scale * 2);

  settingsCursor.allocate((32 + (2 + 3) * 2) * scale, (23 + (2 + 3) * 2) * scale);
  settingsCursor.fill(0x00000000);
  static auto imposeSettingsCursorPart = [&](string id, uint x, uint y) -> void {
    extract({"cursorSettings", id}, cursorPart, false);
    cursorPart.crop(1, 1, 30, 30);
    cursorPart.scale(w, h, false);
    settingsCursor.impose(image::blend::sourceAlpha, x, y, cursorPart, 0, 0, w, h);
  };
  w = 10 * scale;
  h = 10 * scale;
  imposeSettingsCursorPart("1", 0,                          0);
  imposeSettingsCursorPart("3", settingsCursor.width() - w, 0);
  imposeSettingsCursorPart("7", 0,                          settingsCursor.height() - h);
  imposeSettingsCursorPart("9", settingsCursor.width() - w, settingsCursor.height() - h);
  w = settingsCursor.width() - 16 * scale;
  h = 10 * scale;
  imposeSettingsCursorPart("2", 8 * scale, 0);
  imposeSettingsCursorPart("8", 8 * scale, settingsCursor.height() - h);
  w = 10 * scale;
  h = settingsCursor.height() - 16 * scale;
  imposeSettingsCursorPart("4", 0,                          8 * scale);
  imposeSettingsCursorPart("6", settingsCursor.width() - w, 8 * scale);
  extract("cursorSettingsBack", cursorPart, false);
  w = 32 * scale;
  h = 23 * scale;
  cursorPart.scale(w, h, false);
  settingsCursor.impose(image::blend::sourceAlpha, 5 * scale, 5 * scale, cursorPart, 0, 0, w, h);

  loadBoxes();
  gameCards.reset();
  for(uint id : range(home->media.size())) gameCards.append(loadGameCard(id, false));

  bool audioValid = false;
  if(bgm = vfs::fs::file::open(locate({path, "bgm.pcm"}), vfs::file::mode::read)) {
    if(bgm->size() >= 8) {
      uint32 header = bgm->readm(4);
      if(header == 0x4d535531) {  //"MSU1"
        audioValid = true;
      }
    }
  }
  if(!audioValid) bgm.reset();

  font.load();
}

auto Theme::loadBoxes() -> void {
  boxes.reset();
  for(uint mediumID : range(home->media.size())) {
    image& box = boxes(mediumID);
    box.load(string{home->media[mediumID].path(), "box.png"});

    uint boundX = 2 * graphics->scale;
    uint boundY = 2 * graphics->scale;
    uint boundW = gameCardBase.width () - boundX -  4 * graphics->scale;
    uint boundH = gameCardBase.height() - boundY - 20 * graphics->scale;

    uint rectW = boundW;
    uint rectH = boundH;
    if(box.width() > box.height() * boundW / boundH) {
      rectH = box.height() * boundW / box.width();
    } else {
      rectW = box.width() * boundH / box.height();
    }
    box.scale(rectW, rectH, true);
  }
}

auto Theme::loadGameCard(uint mediumID, bool active) -> image {
  image card{active ? gameCardActiveBase : gameCardBase};

  image& box = boxes[mediumID];
  if(box) {
    uint boundX = 2 * graphics->scale;
    uint boundY = 2 * graphics->scale;
    uint boundW = card.width () - boundX -  4 * graphics->scale;
    uint boundH = card.height() - boundY - 20 * graphics->scale;

    uint rectX = boundX + boundW / 2 - box.width () / 2;
    uint rectY = boundY + boundH / 2 - box.height() / 2;
    card.impose(image::blend::sourceColor, rectX, rectY, box, 0, 0, box.width(), box.height());
  }

  return card;
}

auto Theme::updateActiveGameCard(uint cursor) -> void {
  gameCardActive = loadGameCard(cursor, true);
}
