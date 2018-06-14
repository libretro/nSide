//shimai
//From the term 「四枚の葉」, which means "four-leaf clover", named after the
//NES Classic Edition's code name "Clover".

#include <nall/nall.hpp>
#include <ruby/ruby.hpp>
#include <hiro/hiro.hpp>
using namespace nall;
using namespace ruby;
using namespace hiro;
extern unique_pointer<Video> video;
extern unique_pointer<Audio> audio;
extern unique_pointer<Input> input;

#include <emulator/emulator.hpp>
extern Emulator::Interface* emulator;

#include "program/program.hpp"
#include "configuration/configuration.hpp"
#include "input/input.hpp"
#include "graphics/graphics.hpp"
#include "sound/sound.hpp"
#include "controls/controls.hpp"
#include "theme/theme.hpp"
#include "scene/scene.hpp"
#include "home/home.hpp"
#include "presentation/presentation.hpp"

auto locate(string name) -> string;
auto locateSystem(string name) -> string;
