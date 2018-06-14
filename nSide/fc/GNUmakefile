processors += mos6502 z80

objects += fc-interface fc-system fc-controller
objects += fc-cartridge fc-memory
objects += fc-cpu fc-apu fc-ppu
objects += fc-expansion
objects += fc-vssystem fc-playchoice10 fc-famicombox

obj/fc-interface.o:    fc/interface/interface.cpp $(call rwildcard,fc/interface/)
obj/fc-system.o:       fc/system/system.cpp $(call rwildcard,fc/system/)
obj/fc-controller.o:   fc/controller/controller.cpp $(call rwildcard,fc/controller/)
obj/fc-cartridge.o:    fc/cartridge/cartridge.cpp $(call rwildcard,fc/cartridge/)
obj/fc-memory.o:       fc/memory/memory.cpp $(call rwildcard,fc/memory/)

obj/fc-cpu.o:          fc/cpu/cpu.cpp $(call rwildcard,fc/cpu/)
obj/fc-apu.o:          fc/apu/apu.cpp $(call rwildcard,fc/apu/)
obj/fc-ppu.o:          fc/ppu/ppu.cpp $(call rwildcard,fc/ppu/)

obj/fc-expansion.o:    fc/expansion/expansion.cpp $(call rwildcard,fc/expansion/)

obj/fc-vssystem.o:     fc/arcade/vssystem/vssystem.cpp $(call rwildcard,fc/arcade/vssystem/)
obj/fc-playchoice10.o: fc/arcade/playchoice10/playchoice10.cpp $(call rwildcard,fc/arcade/playchoice10/)
obj/fc-famicombox.o:   fc/arcade/famicombox/famicombox.cpp $(call rwildcard,fc/arcade/famicombox/)
