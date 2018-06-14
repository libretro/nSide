struct Home : Scene {
  Home();

  auto reset() -> void;
  auto run() -> void;

  auto loadSprites(vector<Sprite*>& sprites) -> void;
  auto updateSprites() -> void;

  struct Medium {
    auto path() -> string;

    string basename;
    string name;
    string title;
  };
  vector<Medium> media;

  auto cursorUp() -> void;
  auto cursorDown() -> void;
  auto cursorLeft() -> void;
  auto cursorRight() -> void;
  auto confirm() -> void;
  auto loadMedium() -> void;

  auto medium() -> Medium&;

  string system;

private:
  auto setCursor(int cursor) -> void;
  auto cursorReady() -> bool;

  uint gameCursor;
  double gameScroll;

  uint settingsCursor;

  double cursorPosition;
  bool settingsActive;

  Sprite menubarU;
  Sprite menubarL;
  Sprite captionTitle;
  Sprite captionTitleContents;
  image captionTitleText;
  vector<Sprite> gameCards;
  vector<Sprite> settingsButtons;
  Sprite cursorSprite;
};

extern unique_pointer<Home> home;
