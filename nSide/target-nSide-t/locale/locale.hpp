struct Locale {
  auto load(string name) -> void;
  auto refresh() -> void;

  auto operator[](const string& path) -> string;

private:
  Markup::Node document;
};

extern Locale locale;
