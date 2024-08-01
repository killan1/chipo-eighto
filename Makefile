VPATH = src
objects = chipo-eighto.o chip.o media.o utils.o sys.o args.o config.o

chipo8o: $(objects)
	cc -g3 --std=c99 -Wall -Wextra -Wpedantic -fsanitize=address,undefined -o chipo8o $(objects) -lraylib -lm

chipo-eighto.o: chipo-eighto.c 
chip.o: chip.h
media.o: media.h
utils.o: utils.h chip.h sys.h media.h
sys.o: sys.h
args.o: args.h
config.o: config.h media.h utils.h

.PHONY: clean
clean:
	-rm $(objects)
