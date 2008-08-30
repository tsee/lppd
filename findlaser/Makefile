INCDIR=include
LIBDIR=lib
BINDIR=bin

LDFLAGS += -ggdb -ljpeg -fPIC -lstdc++ -lX11
SDL_LDFLAGS= -lSDL_image -lSDL_gfx -lSDL_ttf $(shell sdl-config --libs) 
CXXFLAGS += -ggdb -fPIC $(shell sdl-config --cflags) -Isrc
#CXXFLAGS += -pg
CXXFLAGS += -O2

SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj

SOURCES=$(wildcard $(SRC_DIR)/*.cc)
HEADERS=$(wildcard $(SRC_DIR)/*.h))
OBJECTS=$(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SOURCES))
#OBJECTS+=$(patsubst %ROOT.h, %Dict.o, $(wildcard */*ROOT.h))

EXEBASE=sdl findLaser findLaserCapture testGeoTrans geoCalib
EXE=$(patsubst %,$(BIN_DIR)/%, $(EXEBASE))

IMAGE_OBJECTS=$(patsubst /%, $(OBJ_DIR)/%.o,   /Color /Image /jpegstuff /GreyImage /ColorImage)

HISTORY_OBJECTS=$(patsubst /%, $(OBJ_DIR)/%.o, /HistoryPoint /History)

COMMON_OBJECTS=$(patsubst /%, $(OBJ_DIR)/%.o,  /Configuration /ImageCapture /GeometryCorrection) $(HISTORY_OBJECTS)

all: $(OBJECTS) bin

$(OBJ_DIR)/%.o: $(SOURCES)
	$(CXX) -o $@ -c $(SRC_DIR)/$*.cc $(CXXFLAGS)

nonbin: $(OBJECTS)

bin: nonbin $(EXE)

$(BIN_DIR)/sdl: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(OBJ_DIR)/sdl.o -o $@ $(LDFLAGS) $(SDL_LDFLAGS)

$(BIN_DIR)/geoCalib: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(OBJ_DIR)/geoCalib.o -o $@ $(LDFLAGS) $(SDL_LDFLAGS)

$(BIN_DIR)/findLaser: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(OBJ_DIR)/findLaser.o -o $@ $(LDFLAGS)

$(BIN_DIR)/findLaserCapture: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(OBJ_DIR)/findLaserCapture.o -o $@ $(LDFLAGS)

$(BIN_DIR)/testGeoTrans: nonbin
	$(CXX) $(CXXFLAGS) $(COMMON_OBJECTS) $(OBJ_DIR)/testGeoTrans.o -o $@ $(LDFLAGS)

clean:
	@rm -f $(OBJECTS) *~ core *.o *.so $(OBJ_DIR)/*.o

realclean:
	@rm -f $(OBJECTS) *~ core *.o *.so $(OBJ_DIR)/*.o $(BIN_DIR)/*

.PHONY: all clean nonbin bin realclean
