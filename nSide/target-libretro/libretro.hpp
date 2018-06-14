#include "libretro.h"
#include <nall/nall.hpp>
#include <emulator/emulator.hpp>

static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_input_poll_t input_poll;
static retro_input_state_t input_state;
static retro_log_printf_t libretro_print;

auto locate_libretro(string name) -> string;

#include "program.hpp"
#include "icarus.hpp"
#include "system.hpp"