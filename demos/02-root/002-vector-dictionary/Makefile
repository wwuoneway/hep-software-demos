ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS   := $(shell root-config --libs)
# Extra flags, e.g. EXTRA_CXXFLAGS=-I<vdt>/include when ROOT's RVec.hxx cannot find vdt/vdtMath.h.
EXTRA_CXXFLAGS ?=
B := build

.PHONY: all run clean

all: $(B)/probe

# Dictionary for Point, std::vector<Point> and Pixel (not std::vector<Pixel>), see LinkDef.h.
$(B)/ShapesDict.cxx: include/Shapes.h include/LinkDef.h
	@mkdir -p $(B)
	rootcling -f $@ -rmf $(B)/libShapes.rootmap -rml libShapes.so -I include Shapes.h LinkDef.h

$(B)/libShapes.so: $(B)/ShapesDict.cxx
	clang++ -std=c++20 -shared -fPIC $(ROOT_CFLAGS) $(EXTRA_CXXFLAGS) -I include $< -o $@ $(ROOT_LIBS)

$(B)/probe: src/probe.cpp include/Shapes.h $(B)/libShapes.so
	clang++ -std=c++20 $(ROOT_CFLAGS) $(EXTRA_CXXFLAGS) -I include src/probe.cpp -o $@ -L$(B) -lShapes $(ROOT_LIBS) -Wl,-rpath,$(abspath $(B))

run: $(B)/probe
	./scripts/run_all.sh

clean:
	rm -rf $(B)
