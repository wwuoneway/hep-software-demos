ROOT_CFLAGS := $(shell root-config --cflags)
ROOT_LIBS   := $(shell root-config --libs)
B := build

.PHONY: all with-dict no-dict run-with-dict run-no-dict read-no-lib read-with-lib clean

all: with-dict no-dict

# 1. generate the dictionary, 2. build it into a shared lib (+ .rootmap and _rdict.pcm)
$(B)/EventDict.cxx: include/Event.h include/LinkDef.h
	@mkdir -p $(B)
	rootcling -f $@ -rmf $(B)/libEvent.rootmap -rml libEvent.so -I include Event.h LinkDef.h

$(B)/libEvent.so: $(B)/EventDict.cxx
	clang++ -shared -fPIC $(ROOT_CFLAGS) -I include $< -o $@ $(ROOT_LIBS)

with-dict: $(B)/libEvent.so src/write_event.cpp
	clang++ $(ROOT_CFLAGS) -I include src/write_event.cpp -o $(B)/write_with_dict -L$(B) -lEvent $(ROOT_LIBS) -Wl,-rpath,$(abspath $(B))

# Same source, but no dictionary linked in.
no-dict: src/write_event.cpp
	@mkdir -p $(B)
	clang++ $(ROOT_CFLAGS) -I include src/write_event.cpp -o $(B)/write_no_dict $(ROOT_LIBS)

run-with-dict: with-dict
	$(B)/write_with_dict $(B)/events.root

run-no-dict: no-dict
	-$(B)/write_no_dict $(B)/events_nodict.root

read-no-lib:
	root -l -b -q scripts/read_without_lib.C

read-with-lib:
	ROOT_INCLUDE_PATH=include root -l -b -q scripts/read_with_lib.C

clean:
	rm -rf $(B)
