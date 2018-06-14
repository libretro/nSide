#include "../shimai.hpp"
unique_pointer<Home> home;

#define maxVisibleCards 6
#define maxSelectableCards 4  //only used with looping list

#define trueFmod(n, m)\
  fmod((fmod(n,m)+m),m)
#define trueMod(n, m)\
  (n%m+m)%m

Home::Home() {
  home = this;

  auto shimaiSettings = BML::unserialize(file::read(locate("shimai.bml")));

  system = shimaiSettings["shimai/system"].text();
  string type;
  for(auto& emulator : program->emulators) {
    if(emulator->information.name == system) {
      type = emulator->media[0].type;
      break;
    }
  }

  media.reset();
  string consolePath = {settings["Library/Location"].text(), system, "/"};
  auto gameList = directory::folders(consolePath, {"*.", type});
  for(string basename : gameList) {
    Medium game;
    game.basename = basename;
    if(!file::exists({game.path(), "box.png"})) continue;

    string markup;
    string manifestPath = {game.path(), "manifest.bml"};
    if(!file::exists(manifestPath) || settings["Library/IgnoreManifests"].boolean()) {
      if(auto manifest = execute("cart-pal", "--manifest", game.path())) {
        markup = manifest.output;
      }
    } else {
      markup = file::read(manifestPath);
    }
    auto manifest = BML::unserialize(markup);
    game.name = manifest["information/name"] ? manifest["information/name"].text() : basename;
    game.title = manifest["information/title"] ? manifest["information/title"].text() : basename;

    media.append(game);
  }

  graphics->initialize();

  theme->load(shimaiSettings["shimai/theme"].text());

  gameCursor = 0;
  gameScroll = 0.0;

  settingsCursor = 0;

  cursorPosition = gameCursor;
  settingsActive = false;
}

auto Home::reset() -> void {
  graphics->reset();
  sound->reset();
  controls->reset();

//controls->onPressUp    = {&Home::cursorUp,    this};
//controls->onPressDown  = {&Home::cursorDown,  this};
  controls->onPressLeft  = {&Home::cursorLeft,  this};
  controls->onPressRight = {&Home::cursorRight, this};
  controls->onPressA     = {&Home::confirm, this};
  controls->onPressStart = {&Home::loadMedium, this};
}

auto Home::run() -> void {
  graphics->run();
  sound->run();
  if(graphics->step == 0) controls->run();
}

auto Home::loadSprites(vector<Sprite*>& sprites) -> void {
  menubarU.img = &theme->menubarU;
  menubarU.set(0.5, 0.0, 0.5, (int)(8 * graphics->scale));

  menubarL.img = &theme->menubarL;
  menubarL.set(0.5, 1.0, 0.5, menubarL.height() - (int)(10 * graphics->scale));

  captionTitle.set(0.5, 0.25, 0.5, 0.5, theme->captionTitle);
  captionTitleText.allocate(captionTitle.width(), captionTitle.height());
  captionTitleContents.set(0.5, 0.25, 0.5, 0.5, captionTitleText);

  gameCards.reset();
  for(uint i : range(min(media.size(), maxVisibleCards))) {
    gameCards.append(Sprite(0.5, 0.5, 0.5, 0.5, theme->gameCardBase));
  }

  settingsButtons.reset();
  settingsButtons.append(Sprite(0, 62.0 / 720.0, 0.5, 0.5, theme->settingsDisplay));
  settingsButtons.append(Sprite(0, 62.0 / 720.0, 0.5, 0.5, theme->settingsOption));

  for(int i : range(settingsButtons.size())) {
    auto& button = settingsButtons[i];
    int offset = i * 32 - ((int)settingsButtons.size() - 1) * 16;
    button.x = graphics->buffer.width() / 2 + offset * graphics->scale;
  }

  cursorSprite.set(0, 0.5, 0.5, 0.5, theme->gameCursor);

  setCursor(gameCursor);

  sprites.append(&menubarU);
  sprites.append(&menubarL);
  sprites.append(&captionTitle);
  sprites.append(&captionTitleContents);
  for(auto& card : gameCards) sprites.append(&card);
  sprites.append(&cursorSprite);
//for(auto& button : settingsButtons) sprites.append(&button);
};

auto Home::updateSprites() -> void {
  menubarU.oy = (settingsActive ? 6 : 8) * graphics->scale;

  const double scrollRate = 1.0 / 8.0;

  double targetCursorPosition;

  if(!settingsActive) {
    if(media.size() >= maxVisibleCards) {
      //Why trueFmod is needed: When the first game is selected while the last
      //game is onscreen, gameScroll will be a high value, and gameCursor will
      //be 0, making the dividend a negative number.
      targetCursorPosition = trueFmod(gameCursor + 1.0 - gameScroll, media.size()) - 1.0;
      if(targetCursorPosition > maxSelectableCards - 1.0) {
        gameScroll = fmod(gameScroll + scrollRate, media.size());
        cursorPosition -= scrollRate;
      } else if(targetCursorPosition < 0.0) {
        gameScroll = fmod(gameScroll + media.size() - scrollRate, media.size());
        cursorPosition += scrollRate;
      } else if(gameScroll != round(gameScroll)) {
        gameScroll = round(gameScroll);
        cursorPosition = round(cursorPosition);
      }
    } else {
      targetCursorPosition = gameCursor;
    }
    if(cursorPosition < targetCursorPosition - scrollRate) {
      cursorPosition += scrollRate;
    } else if(cursorPosition > targetCursorPosition + scrollRate) {
      cursorPosition -= scrollRate;
    } else if(cursorPosition != targetCursorPosition) {
      cursorPosition = targetCursorPosition;
    }

    double cardX;
    double cursorX;
    if(media.size() >= maxVisibleCards) {
      for(int i : range(gameCards.size())) {
        auto& gameCard = gameCards[i];

        //Why trueMod is needed: When gameScroll is 0, the first 5 games will
        //be onscreen as well as the very last game at the far-left. "Very last"
        //is represented by -1 here.
        uint index = trueMod(i - 1 + (int)(gameScroll + 0.5), media.size());
        cardX = (i + 0.5 - fmod(gameScroll + 0.5, 1.0)) / (maxVisibleCards - 1);
        gameCard.x = cardX * graphics->buffer.width();

        bool selected = cursorReady() && index == gameCursor;
        gameCard.img = selected ? &theme->gameCardActive : &theme->gameCards[index];
      }
      cursorX = (cursorPosition + 1.0) / (maxVisibleCards - 1);
    } else {
      for(int i : range(gameCards.size())) {
        auto& gameCard = gameCards[i];

        cardX = (i + (maxVisibleCards / 2.0) - media.size() * 0.5) / (maxVisibleCards - 1);
        gameCard.x = cardX * graphics->buffer.width();

        bool selected = cursorReady() && i == gameCursor;
        gameCard.img = selected ? &theme->gameCardActive : &theme->gameCards[i];
      }
      cursorX = (cursorPosition + (maxVisibleCards / 2.0) - media.size() * 0.5) / (maxVisibleCards - 1);
    }
    cursorSprite.x = cursorX * graphics->buffer.width();
  } else {
    targetCursorPosition = settingsCursor;
    if(cursorPosition < targetCursorPosition - scrollRate) {
      cursorPosition += scrollRate;
    } else if(cursorPosition > targetCursorPosition + scrollRate) {
      cursorPosition -= scrollRate;
    } else if(cursorPosition != targetCursorPosition) {
      cursorPosition = targetCursorPosition;
    }

    cursorSprite.x = settingsButtons[0].x + cursorPosition * 32 * graphics->scale;
  }
};

auto Home::cursorUp() -> void {
  if(cursorReady() && !settingsActive) {
    settingsActive = true;
    cursorPosition = settingsCursor;
    cursorSprite.set(0, (int)(6 * graphics->scale), 0.5, 0, theme->settingsCursor);
    for(auto& button : settingsButtons) button.y = 68.0 / 720.0 * graphics->buffer.height();
  }
};

auto Home::cursorDown() -> void {
  if(cursorReady() && settingsActive) {
    settingsActive = false;
    //Why trueFmod is needed: When the first game is selected while the last
    //game is onscreen, gameScroll will be a high value, and gameCursor will
    //be 0, making the dividend a negative number.
    cursorPosition = trueFmod(gameCursor + 1.0 - gameScroll, media.size()) - 1.0;
    cursorSprite.set(0, 0.5, 0.5, 0.5, theme->gameCursor);
    for(auto& button : settingsButtons) button.y = 62.0 / 720.0 * graphics->buffer.height();
  }
};

auto Home::cursorLeft() -> void {
  if(!cursorReady()) return;
  if(!settingsActive) setCursor((gameCursor + media.size() - 1) % media.size());
  if( settingsActive) setCursor((settingsCursor + settingsButtons.size() - 1) % settingsButtons.size());
};

auto Home::cursorRight() -> void {
  if(!cursorReady()) return;
  if(!settingsActive) setCursor((gameCursor + 1) % media.size());
  if( settingsActive) setCursor((settingsCursor + 1) % settingsButtons.size());
};

auto Home::confirm() -> void {
  if(!cursorReady()) return;
  if(!settingsActive) {
    loadMedium();
  } else {
  }
};

auto Home::loadMedium() -> void {
  if(cursorReady() && !settingsActive) {
    program->mediumQueue.append(medium().path());
    program->loadMedium();
    program->updateVideoShader();
  }
};

auto Home::medium() -> Medium& {
  return media[gameCursor];
}

auto Home::setCursor(int cursor) -> void {
  if(!settingsActive) {
    gameCursor = cursor;
    theme->updateActiveGameCard(cursor);
    graphics->drawTextCenter(medium().title, captionTitleText);
  } else {
    settingsCursor = cursor;
  }
};

auto Home::cursorReady() -> bool {
  if(!settingsActive) {
    return fmod(gameScroll + cursorPosition, media.size()) == gameCursor;
  } else {
    return cursorPosition == settingsCursor;
  }
};

auto Home::Medium::path() -> string {
  return {settings["Library/Location"].text(), home->system, "/", basename};
}

#undef trueFmod
#undef trueMod

#undef maxVisibleCards
#undef maxSelectableCards
