# 
# Author: Daniel Liscinsky
# 

CC = gcc
CPP = g++

FLAGS = -O3 -g
LNKFLAGS = -s


OUT_DIR = ../x64/Debug/
PROGS = $(OUT_DIR)/BitTorrent_Client
OBJ_FILES = $(OUT_DIR)/bencode.o $(OUT_DIR)/bittorrent.o \
		$(OUT_DIR)/sha1.o $(OUT_DIR)/memutil.o


.PHONY: all clean


all: $(PROGS)


$(OUT_DIR)/BitTorrent_Client: bittorrent_main.cpp $(OBJ_FILES)
	$(CPP) $(LNKFLAGS) $^ -o $@ -std=c++11 -lpthread

$(OUT_DIR)/bencode.o: bencode.cpp bencode.h
	$(CPP) $(FLAGS) -c bencode.cpp -o $@ -std=c++11

$(OUT_DIR)/bittorrent.o: bittorrent.cpp bittorrent.h
	$(CPP) $(FLAGS) -c bittorrent.cpp -o $@ -std=c++11

$(OUT_DIR)/sha1.o: sha1.c sha1.h
	$(CC) $(FLAGS) -c sha1.c -o $@

$(OUT_DIR)/memutil.o: memutil.c memutil.h
	$(CC) $(FLAGS) -c memutil.c -o $@


clean:
	rm -f $(PROGS) $(OUT_DIR)/*.o
