processors += mos6502

objects += a2600-interface
objects += a2600-cpu a2600-pia a2600-tia
objects += a2600-system a2600-cartridge
objects += a2600-controller

obj/a2600-interface.o:    a2600/interface/interface.cpp $(call rwildcard,a2600/interface/)
obj/a2600-cpu.o:          a2600/cpu/cpu.cpp $(call rwildcard,a2600/cpu/)
obj/a2600-pia.o:          a2600/pia/pia.cpp $(call rwildcard,a2600/pia/)
obj/a2600-tia.o:          a2600/tia/tia.cpp $(call rwildcard,a2600/tia/)
obj/a2600-system.o:       a2600/system/system.cpp $(call rwildcard,a2600/system/)
obj/a2600-cartridge.o:    a2600/cartridge/cartridge.cpp $(call rwildcard,a2600/cartridge/)
obj/a2600-controller.o:   a2600/controller/controller.cpp $(call rwildcard,a2600/controller/)
