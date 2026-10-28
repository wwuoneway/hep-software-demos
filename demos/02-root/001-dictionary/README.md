# ROOT dictionary

## What is it?

C++ has no runtime reflection. A ROOT **dictionary** is generated C++ code (by `rootcling`) that
describes a class: its name, data members, types, offsets, base classes, and how to construct,
destroy and stream it. It is compiled into a shared library, along with:

- `libEvent.rootmap`: maps class names to the library, so ROOT can auto-load it.
- `EventDict_rdict.pcm`: precompiled module/reflection info used by Cling.

## Why is it needed?

ROOT I/O, `TTree`, `TBranch`, Cling and PyROOT all work with classes **by name at runtime**.
To split `Event` into branches, write it to disk, and read it back, ROOT must know the layout of
`Event` and `std::vector<Particle>`. Only the dictionary provides that.

## Layout

| File | Role |
|---|---|
| `include/Event.h` | `Particle` and `Event` classes (plain data) |
| `include/LinkDef.h` | tells `rootcling` which classes get dictionaries (`+` = generate streamer, schema evolution) |
| `src/write_event.cpp` | writes `Event` into a `TTree` |
| `scripts/read_without_lib.C` | reads the file without `libEvent` |
| `scripts/read_with_lib.C` | reads the file with `libEvent` and real `Event` type |

## Run

```bash
setup_root_local        # source thisroot.sh
make run-with-dict      # rootcling -> libEvent.so -> write_with_dict
make run-no-dict        # same source, no dictionary
make read-no-lib
make read-with-lib
make clean
```

## Steps to add a dictionary

1. Write the class in a header.
2. Write `LinkDef.h` with `#pragma link C++ class ...+;` (including `std::vector<Particle>`).
3. `rootcling -f EventDict.cxx -rmf libEvent.rootmap -rml libEvent.so -I include Event.h LinkDef.h`
4. Compile `EventDict.cxx` into `libEvent.so` and link/load it.

(In a CMake project, `ROOT_GENERATE_DICTIONARY` does steps 3-4 for you.)

## What happens without a dictionary?

`make run-no-dict` builds the same `write_event.cpp` without the library:

```
Error in <TTree::Branch>: The pointer specified for event is not of a class or type known to ROOT
```

The program still compiles and runs: the error is only a runtime message, `Branch` returns null,
and the tree is written with **no `event` branch**, so the output is silently useless. Compilation
success says nothing about whether ROOT can handle the class.

## Reading without the library

`make read-no-lib` shows that a file written with a dictionary stores its `TStreamerInfo`. ROOT can
then read the data members using an *emulated* class built from that info (no methods, no
`Event*` in C++). `make read-with-lib` loads `libEvent` and uses the real class.

## Using the dictionary in an interactive `root` session

Two things must be discoverable, and `source env.sh` sets up both:

- **The library**, via `DYLD_LIBRARY_PATH` / `LD_LIBRARY_PATH` (`build/`). ROOT then finds `libEvent.rootmap` and auto-loads `libEvent` when it meets `Event`. Without it you get `Warning in <TClass::Init>: no dictionary for class Event is available`.
- **The header**, via `ROOT_INCLUDE_PATH` (`include/`). The dictionary records the header as `Event.h`, and Cling must be able to resolve it. Without it you get `Missing FileEntry for Event.h` even if `libEvent` is loaded.

```bash
source env.sh
root -l build/events.root     # no warnings
```

Note: PyROOT needs a Python matching the ROOT build (3.11 here), so Python variants are left
for `demos/03-python`.
