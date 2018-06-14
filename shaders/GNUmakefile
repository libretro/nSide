prefix := $(HOME)/.local

install:
	if [ -d $(prefix)/share/nSide/Video\ Shaders ]; then rm -r $(prefix)/share/nSide/Video\ Shaders; fi
	mkdir -p $(prefix)/share/nSide/Video\ Shaders
	cp -r *.shader $(prefix)/share/nSide/Video\ Shaders
	chmod -R 777 $(prefix)/share/nSide/Video\ Shaders
