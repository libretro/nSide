#include "../laevateinn.hpp"
Settings settings;

Settings::Settings() {
  Markup::Node::operator=(BML::unserialize(string::read(locate("settings.bml"))));

  auto set = [&](const string& name, const string& value) {
    //create node and set to default value only if it does not already exist
    if(!operator[](name)) operator()(name).setValue(value);
  };

  set("Video/Synchronize", false);

  set("Audio/Synchronize", true);
  set("Audio/Mute", false);

  //Input
}

auto Settings::quit() -> void {
  file::write(locate("settings.bml"), BML::serialize(*this));
}
