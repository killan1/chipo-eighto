VPATH = src
objects = chipo-eighto.o chip.o media.o utils.o

chipo8o: $(objects)
				cc -g3 --std=c99 -Wall -o chipo8o $(objects) -lraylib

chipo-eighto.o: chipo-eighto.c 
chip.o: chip.h
media.o: media.h
utils.o: utils.h

.PHONY: clean
clean:
				-rm chipo-eighto.o utils.o chip.o media.o
