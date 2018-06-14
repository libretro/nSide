processors += wdc65816 spc700 arm7tdmi gsu hg51b upd96050

objects += sfc-balanced-interface sfc-balanced-system sfc-balanced-controller
objects += sfc-balanced-cartridge sfc-balanced-memory
objects += sfc-balanced-cpu sfc-balanced-smp sfc-balanced-dsp sfc-balanced-ppu
objects += sfc-balanced-expansion sfc-balanced-satellaview sfc-balanced-21fx
objects += sfc-balanced-icd sfc-balanced-mcc sfc-balanced-dip sfc-balanced-event
objects += sfc-balanced-sa1 sfc-balanced-superfx
objects += sfc-balanced-armdsp sfc-balanced-hitachidsp sfc-balanced-necdsp
objects += sfc-balanced-epsonrtc sfc-balanced-sharprtc
objects += sfc-balanced-spc7110 sfc-balanced-sdd1
objects += sfc-balanced-obc1 sfc-balanced-msu1
objects += sfc-balanced-bsmemory sfc-balanced-sufamiturbo

flags += -DPROFILE_BALANCED

obj/sfc-balanced-interface.o:   sfc-balanced/interface/interface.cpp $(call rwildcard,sfc-balanced/interface)
obj/sfc-balanced-system.o:      sfc-balanced/system/system.cpp $(call rwildcard,sfc-balanced/system/)
obj/sfc-balanced-controller.o:  sfc-balanced/controller/controller.cpp $(call rwildcard,sfc-balanced/controller/)
obj/sfc-balanced-cartridge.o:   sfc-balanced/cartridge/cartridge.cpp $(call rwildcard,sfc-balanced/cartridge/)
obj/sfc-balanced-memory.o:      sfc-balanced/memory/memory.cpp $(call rwildcard,sfc-balanced/memory/)

obj/sfc-balanced-cpu.o:         sfc-balanced/cpu/cpu.cpp $(call rwildcard,sfc-balanced/cpu/)
obj/sfc-balanced-smp.o:         sfc-balanced/smp/smp.cpp $(call rwildcard,sfc-balanced/smp/)
obj/sfc-balanced-dsp.o:         sfc-balanced/dsp/dsp.cpp $(call rwildcard,sfc-balanced/dsp/)
obj/sfc-balanced-ppu.o:         sfc-balanced/ppu/ppu.cpp $(call rwildcard,sfc-balanced/ppu/)

obj/sfc-balanced-expansion.o:   sfc-balanced/expansion/expansion.cpp $(call rwildcard,sfc-balanced/expansion/)
obj/sfc-balanced-satellaview.o: sfc-balanced/expansion/satellaview/satellaview.cpp $(call rwildcard,sfc-balanced/expansion/satellaview/)
obj/sfc-balanced-21fx.o:        sfc-balanced/expansion/21fx/21fx.cpp $(call rwildcard,sfc-balanced/expansion/21fx/)

obj/sfc-balanced-icd.o:         sfc-balanced/coprocessor/icd/icd.cpp $(call rwildcard,sfc-balanced/coprocessor/icd/)
obj/sfc-balanced-mcc.o:         sfc-balanced/coprocessor/mcc/mcc.cpp $(call rwildcard,sfc-balanced/coprocessor/mcc/)
obj/sfc-balanced-dip.o:         sfc-balanced/coprocessor/dip/dip.cpp $(call rwildcard,sfc-balanced/coprocessor/dip/)
obj/sfc-balanced-event.o:       sfc-balanced/coprocessor/event/event.cpp $(call rwildcard,sfc-balanced/coprocessor/event/)

obj/sfc-balanced-sa1.o:         sfc-balanced/coprocessor/sa1/sa1.cpp $(call rwildcard,sfc-balanced/coprocessor/sa1/)
obj/sfc-balanced-superfx.o:     sfc-balanced/coprocessor/superfx/superfx.cpp $(call rwildcard,sfc-balanced/coprocessor/superfx/)

obj/sfc-balanced-armdsp.o:      sfc-balanced/coprocessor/armdsp/armdsp.cpp $(call rwildcard,sfc-balanced/coprocessor/armdsp/)
obj/sfc-balanced-hitachidsp.o:  sfc-balanced/coprocessor/hitachidsp/hitachidsp.cpp $(call rwildcard,sfc-balanced/coprocessor/hitachidsp/)
obj/sfc-balanced-necdsp.o:      sfc-balanced/coprocessor/necdsp/necdsp.cpp $(call rwildcard,sfc-balanced/coprocessor/necdsp/)

obj/sfc-balanced-epsonrtc.o:    sfc-balanced/coprocessor/epsonrtc/epsonrtc.cpp $(call rwildcard,sfc-balanced/coprocessor/epsonrtc/)
obj/sfc-balanced-sharprtc.o:    sfc-balanced/coprocessor/sharprtc/sharprtc.cpp $(call rwildcard,sfc-balanced/coprocessor/sharprtc/)

obj/sfc-balanced-spc7110.o:     sfc-balanced/coprocessor/spc7110/spc7110.cpp $(call rwildcard,sfc-balanced/coprocessor/spc7110/)
obj/sfc-balanced-sdd1.o:        sfc-balanced/coprocessor/sdd1/sdd1.cpp $(call rwildcard,sfc-balanced/coprocessor/sdd1/)
obj/sfc-balanced-obc1.o:        sfc-balanced/coprocessor/obc1/obc1.cpp $(call rwildcard,sfc-balanced/coprocessor/obc1/)

obj/sfc-balanced-msu1.o:        sfc-balanced/coprocessor/msu1/msu1.cpp $(call rwildcard,sfc-balanced/coprocessor/msu1/)

obj/sfc-balanced-bsmemory.o:    sfc-balanced/slot/bsmemory/bsmemory.cpp $(call rwildcard,sfc-balanced/slot/bsmemory/)
obj/sfc-balanced-sufamiturbo.o: sfc-balanced/slot/sufamiturbo/sufamiturbo.cpp $(call rwildcard,sfc-balanced/slot/sufamiturbo/)
