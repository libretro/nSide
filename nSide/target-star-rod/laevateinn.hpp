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

#include <sfc/sfc.hpp>
#undef platform
namespace SFC = SuperFamicom;

auto locate(string name) -> string;
auto locateHigan(string name) -> string;
auto locateSystem(string name) -> string;

#include "program/program.hpp"
#include "configuration/configuration.hpp"
#include "debugger/debugger.hpp"
#include "tracer/tracer.hpp"
#include "console/console.hpp"
#include "presentation/presentation.hpp"
#include "cpu/cpu.hpp"
#include "smp/smp.hpp"
#include "memory/memory.hpp"
#include "breakpoint/breakpoint.hpp"
#include "properties/properties.hpp"
#include "tile/tile.hpp"
#include "bg/bg.hpp"
#include "palette/palette.hpp"
#include "resource/resource.hpp"
