INCDIR=include
LIBDIR=lib
BINDIR=bin

LDFLAGS += -ggdb -ljpeg -fPIC $(shell sdl-config --libs) -lSDL_image -lSDL_gfx -lX11 -lSDL_ttf
CXXFLAGS += -ggdb -fPIC $(shell sdl-config --cflags) -Isrc
#CXXFLAGS += -pg
CXXFLAGS += -O3

SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj

SOURCES=$(wildcard $(SRC_DIR)/*.cc)
HEADERS=$(wildcard $(SRC_DIR)/*.h))
OBJECTS=$(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SOURCES))
#OBJECTS+=$(patsubst %ROOT.h, %Dict.o, $(wildcard */*ROOT.h))

EXEBASE=sdl findLaser findLaserCapture
EXE=$(patsubst %,$(BIN_DIR)/%, $(EXEBASE))

all: $(OBJECTS) bin

$(OBJ_DIR)/%.o: $(SOURCES)
	$(CXX) -o $@ -c $(SRC_DIR)/$*.cc $(CXXFLAGS)

nonbin: $(OBJECTS)

bin: nonbin $(EXE)
#sdl

$(BIN_DIR)/sdl: nonbin
	$(CXX) $(CXXFLAGS) $(OBJ_DIR)/sdl.o $(OBJ_DIR)/myimages.o $(OBJ_DIR)/v4lcapture.o -o $@ $(LDFLAGS)

$(BIN_DIR)/findLaser: nonbin
	$(CXX) $(CXXFLAGS) $(OBJ_DIR)/findLaser.o $(OBJ_DIR)/myimages.o $(OBJ_DIR)/v4lcapture.o -o $@ $(LDFLAGS)

$(BIN_DIR)/findLaserCapture: nonbin
	$(CXX) $(CXXFLAGS) $(OBJ_DIR)/findLaserCapture.o $(OBJ_DIR)/myimages.o $(OBJ_DIR)/v4lcapture.o -o $@ $(LDFLAGS)

clean:
	@rm -f $(OBJECTS) *~ core *.o *.so $(OBJ_DIR)/*.o

realclean:
	@rm -f $(OBJECTS) *~ core *.o *.so $(OBJ_DIR)/*.o $(BIN_DIR)/*

.PHONY: all clean nonbin bin realclean
