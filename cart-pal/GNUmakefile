build := performance
include ../nall/GNUmakefile
include ../hiro/GNUmakefile

name := cart-pal
flags += -I..

objects := obj/hiro.o
objects += obj/cart-pal.o
objects += $(if $(call streq,$(platform),windows),obj/resource.o)

all: $(objects)
	$(strip $(compiler) -o out/$(name) $(objects) $(link) $(hirolink))
ifeq ($(platform),macos)
	rm -rf out/$(name).app
	mkdir -p out/$(name).app/Contents/MacOS/
	mkdir -p out/$(name).app/Contents/Resources/
	mv out/$(name) out/$(name).app/Contents/MacOS/$(name)
	cp data/$(name).plist out/$(name).app/Contents/Info.plist
	sips -s format icns data/$(name).png --out out/$(name).app/Contents/Resources/$(name).icns
endif

obj/hiro.o: ../hiro/hiro.cpp
	$(compiler) $(hiroflags) -o obj/hiro.o -c ../hiro/hiro.cpp

obj/cart-pal.o: cart-pal.cpp $(call rwildcard,core/) $(call rwildcard,heuristics/) $(call rwildcard,ui/)
	$(compiler) $(cppflags) $(flags) -o obj/cart-pal.o -c cart-pal.cpp

obj/resource.o:
	$(windres) data/$(name).rc obj/resource.o

clean:
ifeq ($(platform),macos)
	rm -rf out/$(name).app
endif
	$(call rm,obj/*)
	$(call rm,out/*)

install:
ifeq ($(platform),macos)
	cp -R out/$(name).app /Applications/$(name).app
else ifneq ($(filter $(platform),linux bsd),)
	mkdir -p $(prefix)/bin/
	mkdir -p $(prefix)/share/applications/
	mkdir -p $(prefix)/share/icons/
	mkdir -p $(prefix)/share/icarus/
	mkdir -p $(prefix)/share/icarus/database/
	mkdir -p $(prefix)/share/icarus/firmware/
	cp out/$(name) $(prefix)/bin/$(name)
	cp data/$(name).desktop $(prefix)/share/applications/$(name).desktop
	cp data/$(name).png $(prefix)/share/icons/$(name).png
	cp -R ../icarus/database/* $(prefix)/share/icarus/database/
	cp -R ../icarus/firmware/* $(prefix)/share/icarus/firmware/
	cp -R database/* $(prefix)/share/icarus/Database/
endif

uninstall:
ifeq ($(platform),macos)
	rm -rf /Applications/$(name).app
else ifneq ($(filter $(platform),linux bsd),)
	rm -f $(prefix)/bin/$(name)
	rm -f $(prefix)/share/applications/$(name).desktop
	rm -f $(prefix)/share/icons/$(name).png
endif
