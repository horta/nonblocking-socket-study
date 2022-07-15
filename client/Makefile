LIBS   = -lev
CFLAGS = -Wall -I/opt/homebrew/opt/libev/include -L/opt/homebrew/opt/libev/lib

SRC=$(wildcard *.c)
HDR=$(wildcard *.h)

# This is the default target, which will be built when 
# you invoke make
.PHONY: all clean

all: client

client: $(SRC) $(HDR)
	cc -o $@ $(SRC) $(CFLAGS) $(LIBS)

clean:
	rm -f client
