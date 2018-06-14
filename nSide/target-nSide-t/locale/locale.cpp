#include "../nSide.hpp"
Locale locale;

auto Locale::load(string name) -> void {
  if(name == "Auto") {
    #if defined(PLATFORM_WINDOWS)
    wchar_t name16[LOCALE_NAME_MAX_LENGTH];
    GetUserDefaultLocaleName(name16, LOCALE_NAME_MAX_LENGTH);
    name = (const char*)utf8_t(name16);
    #else
    string env = getenv("LC_ALL");
    if(!env) env = getenv("LC_MESSAGES");
    if(!env) env = getenv("LANG");
    name = env.split(".").left().replace("_", "-");
    #endif
  }

  while(name) {
    if(file::exists(locate({"Locales/", name, ".bml"}))) break;
    auto parts = name.split("-");
    if(parts.size() < 1) {
      name = "default";
      break;
    }
    parts.removeRight();
    name = parts.merge("-");
  }

  document = BML::unserialize(string::read(locate({"Locales/", name, ".bml"})));
}

auto Locale::refresh() -> void {
  presentation->refreshLocale();
  settingsManager->refreshLocale();
  toolsManager->refreshLocale();
  cheatDatabase->refreshLocale();
}

auto Locale::operator[](const string& path) -> string {
  if(const auto& node = document[path]) return node.text();
  return path;
}
