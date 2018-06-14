build := release
console := true
include ../nall/GNUmakefile

flags += -I. -I..

objects := obj/sourcery.o

all: $(objects)
	$(compiler) -o out/sourcery $(objects) $(link)

obj/sourcery.o: sourcery.cpp
	$(compiler) $(cppflags) $(flags) -o obj/sourcery.o -c sourcery.cpp

clean:
	$(call rm,obj/*)
	$(call rm,out/*)
