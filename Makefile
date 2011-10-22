includes = potato.h master.h player.h
objects = master.o master_util.o player.o  player_util.o common_util.o
sources = master.c master_util.c player.c  player_util.c common_util.c
out = master player

all: $(objects) master player

$(objects): $(includes)

master: 
	cc -o master master.o master_util.o common_util.o

player: 
	cc -o player player.o player_util.o common_util.o

.PHONY: clean 
clean:
	rm -f $(out) $(objects)


