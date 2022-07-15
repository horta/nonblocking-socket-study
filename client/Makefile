LIBS   = -lev -lpthread
CFLAGS = -Wall -I/opt/homebrew/opt/libev/include -L/opt/homebrew/opt/libev/lib -std=c11

SRC=$(wildcard *.c)
HDR=$(wildcard *.h)

# This is the default target, which will be built when 
# you invoke make
.PHONY: all clean

all: h3

h3: $(SRC) $(HDR)
	cc -o $@ $(SRC) $(CFLAGS) $(LIBS)

clean:
	rm -f h3
