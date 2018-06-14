ifeq ($(core),sfc)
	$(error "The accuracy profile is prohibited in nSide. Use higan instead.")
else ifeq ($(core),sfc-balanced)
  corename := sfc_balanced
  include sfc-balanced/GNUmakefile
  include gb/GNUmakefile
  flags += -DSFC_SUPERGAMEBOY
else ifeq ($(core),)
  corename := sfc_balanced
  include sfc-balanced/GNUmakefile
  include gb/GNUmakefile
  flags += -DSFC_SUPERGAMEBOY
endif

flags += -Dlibretro_use_$(corename)

ifeq ($(platform),windows)
  name := higan_$(corename)_libretro.dll
else ifeq ($(platform),macos)
  name := higan_$(corename)_libretro.dylib
else
  name := higan_$(corename)_libretro.so
endif
#link += -Wl,--version-script=target-libretro/link.T -Wl,--no-undefined

#include fc/GNUmakefile
#include ms/GNUmakefile
#include md/GNUmakefile
#include pce/GNUmakefile
#include gba/GNUmakefile
#include ws/GNUmakefile
include processor/GNUmakefile

# rules
#objects := libretro-icarus libretro-program libretro-libretro libretro-$(corename) $(objects)
objects := libretro $(objects)
objects := $(patsubst %,obj/%.o,$(objects))

# TODO: find a way to split the code into 4 linker objects without having
# icarus's Database variables complain of multiple definitions
#obj/libretro-icarus.o: $(ui)/icarus.cpp
#obj/libretro-program.o: $(ui)/program.cpp
#obj/libretro-libretro.o: $(ui)/libretro.cpp
#obj/libretro-$(corename).o: $(ui)/libretro-$(subst _balanced,,$(corename)).cpp

obj/libretro.o: $(ui)/libretro.cpp $(ui)/program.cpp $(ui)/icarus.cpp $(ui)/libretro-$(subst _balanced,,$(corename)).cpp

# targets
build: $(objects)
	$(strip $(compiler) -o out/$(name) $(objects) $(link))

