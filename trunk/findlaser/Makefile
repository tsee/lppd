INCDIR=include
LIBDIR=lib
BINDIR=bin

LDFLAGS += -ggdb -ljpeg -fPIC $(shell sdl-config --libs) -lSDL_image -lSDL_gfx -lX11 -lSDL_ttf
#LDFLAGS += -L../imageproc -lFindLaser 
CXXFLAGS += -ggdb -fPIC $(shell sdl-config --cflags) -I../imageproc/src
#CXXFLAGS += -pg
CXXFLAGS += -O3

SOURCES=$(wildcard *.cc)
HEADERS=$(wildcard *.h))
OBJECTS=$(patsubst %.cc, %.o, $(SOURCES))
#OBJECTS+=$(patsubst %ROOT.h, %Dict.o, $(wildcard */*ROOT.h))

OBJNAME=$(patsubst %.cc, %.o, $(wildcard *.cc))

all: $(OBJNAME) bin

.cc.o:
	$(CXX) -o $@ -c $^ $(CXXFLAGS)

nonbin: $(OBJNAME)

bin: nonbin sdl

sdl: nonbin
	$(CXX) $(CXXFLAGS) sdl.cc ../imageproc/src/*.o -o sdl $(LDFLAGS)

clean:
	@rm -f $(OBJNAME) *~ core *.o *.so 
