INCDIR=include
LIBDIR=lib
BINDIR=bin

LDFLAGS += -ggdb3 -ljpeg -fPIC -lstdc++ -lX11
SDL_LDFLAGS= -lSDL_image -lSDL_gfx -lSDL_ttf $(shell sdl-config --libs) 
CXXFLAGS += -ggdb3 -fPIC $(shell sdl-config --cflags) -Isrc
#CXXFLAGS += -pg
CXXFLAGS += -O2

SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj
TEST_DIR=tests

SOURCES=$(wildcard $(SRC_DIR)/*.cc)
HEADERS=$(wildcard $(SRC_DIR)/*.h))
OBJECTS=$(patsubst $(SRC_DIR)/%.cc, $(OBJ_DIR)/%.o, $(SOURCES))
#OBJECTS+=$(patsubst %ROOT.h, %Dict.o, $(wildcard */*ROOT.h))

TESTSOURCES=$(wildcard $(TEST_DIR)/*.cc)
TESTEXE=$(patsubst $(TEST_DIR)/%.cc,$(TEST_DIR)/%, $(TESTSOURCES))

EXEBASE=sdl findLaser findLaserCapture geoCalib
EXE=$(patsubst %,$(BIN_DIR)/%, $(EXEBASE))

IMAGE_OBJECTS=$(patsubst /%, $(OBJ_DIR)/%.o,   /Color /Image /jpegstuff /GreyImage /ColorImage)
HISTORY_OBJECTS=$(patsubst /%, $(OBJ_DIR)/%.o, /HistoryPoint /History)
COMMON_OBJECTS=$(patsubst /%, $(OBJ_DIR)/%.o,  /Configuration /ImageCapture /GeometryCorrection) $(HISTORY_OBJECTS)
GUI_OBJECTS=$(patsubst /%, $(OBJ_DIR)/%.o,  /Cursor)

all: $(OBJECTS) bin test

$(OBJ_DIR)/%.o: $(SOURCES)
	$(CXX) -o $@ -c $(SRC_DIR)/$*.cc $(CXXFLAGS)

nonbin: $(OBJECTS)

bin: nonbin $(EXE)

test: nonbin $(TESTEXE)

# EXECUTABLES
$(BIN_DIR)/sdl: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(GUI_OBJECTS) $(OBJ_DIR)/sdl.o -o $@ $(LDFLAGS) $(SDL_LDFLAGS)

$(BIN_DIR)/geoCalib: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(GUI_OBJECTS) $(OBJ_DIR)/geoCalib.o -o $@ $(LDFLAGS) $(SDL_LDFLAGS)

$(BIN_DIR)/findLaser: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(OBJ_DIR)/findLaser.o -o $@ $(LDFLAGS)

$(BIN_DIR)/findLaserCapture: nonbin
	$(CXX) $(CXXFLAGS) $(IMAGE_OBJECTS) $(COMMON_OBJECTS) $(OBJ_DIR)/findLaserCapture.o -o $@ $(LDFLAGS)

# TESTS
$(TEST_DIR)/testGeoTrans: nonbin
	$(CXX) $(CXXFLAGS) $(COMMON_OBJECTS) $(TEST_DIR)/testGeoTrans.cc -o $@ $(LDFLAGS)

clean:
	@rm -f $(OBJECTS) *~ core *.o *.so $(OBJ_DIR)/*.o

realclean:
	@rm -f $(OBJECTS) *~ core *.o *.so $(OBJ_DIR)/*.o $(BIN_DIR)/* $(TESTEXE)

.PHONY: all clean nonbin bin realclean test

