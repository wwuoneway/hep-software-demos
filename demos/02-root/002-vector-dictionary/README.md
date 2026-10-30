# `std::vector<T>` and ROOT dictionaries: TTree vs RNTuple

## Question

A data product is often `std::vector<T>`. Which dictionaries must exist to write and read it?

- the dictionary for `T` only, or
- the dictionary for `T` **and** for `std::vector<T>`?

The answer depends on the storage backend, so a dictionary check that is correct for one backend
can be wrong for the other.

## Cases

`include/LinkDef.h` gives each struct a different amount of dictionary:

| Type | Dictionary for `T` | Dictionary for `std::vector<T>` |
|---|---|---|
| `std::vector<demo::Point>` | yes | yes |
| `std::vector<demo::Pixel>` | yes | **no** |
| `std::vector<demo::Ghost>` | no (header is visible to the interpreter, so it has an *interpreted* TClass) | no |
| `std::vector<Secret>` | no (defined only in `probe.cpp`, unknown to ROOT) | no |

For each type, `src/probe.cpp` runs, each in its own process:

- **two dictionary checks**:
  - *top-level*: `TClass::GetClass(typeid(std::vector<T>))->HasDictionary()`
  - *RField walk*: create the RNTuple field and require a dictionary for every class in the field
    tree that is not an STL collection (RNTuple handles STL collections itself)
- **TTree**: write the vector, read it back, compare
- **RNTuple**: write the vector, read it back, compare

## Run

```bash
setup_root_local        # source thisroot.sh
make                    # rootcling -> libShapes.so -> build/probe
make run                # prints the table below
make clean
```

If ROOT's `RVec.hxx` cannot find `vdt/vdtMath.h`, pass its include directory:
`make EXTRA_CXXFLAGS=-I<vdt-prefix>/include`.

## Result (ROOT 6.40)

| Type | top-level check | RField walk | TTree | RNTuple |
|---|---|---|---|---|
| `std::vector<demo::Point>` | accept | accept | write + read OK | write + read OK |
| `std::vector<demo::Pixel>` | **REJECT** | accept | `Branch()` refuses | **write + read OK** |
| `std::vector<demo::Ghost>` | REJECT | REJECT (`demo::Ghost`) | `Branch()` refuses | write + read OK (interpreted only) |
| `std::vector<Secret>` | REJECT | REJECT | `Branch()` refuses | write fails: `unknown type: Secret` |

TTree's message for `Pixel`, `Ghost` and `Secret`:

```
Error in <TTree::Branch>: The class requested (vector<demo::Pixel>) for the branch "v" is an instance of
an stl collection and does not have a compiled CollectionProxy. Please generate the dictionary for
this collection (vector<demo::Pixel>) to avoid to write corrupted data.
```

## What it shows

1. **TTree needs a dictionary for the full type**, including `std::vector<T>` itself. This is ROOT's
   own rule: `TTree::Branch` refuses rather than risk writing corrupted data. For TTree, checking
   `HasDictionary()` on the top-level type matches what ROOT requires. To store
   `std::vector<demo::Pixel>` in a TTree, add `#pragma link C++ class std::vector<demo::Pixel>+;`.

2. **RNTuple handles `std::vector<T>` natively** and needs a dictionary only for the user class `T`.
   `std::vector<demo::Pixel>` round-trips although `HasDictionary()` is false for the vector. A top-level
   `HasDictionary()` check would reject it, so it is **too strict for RNTuple**.

3. **The right RNTuple check follows RNTuple's own schema.** Walking the created `RField` and checking
   each non-collection class accepts `Pixel` and rejects `Ghost` and `Secret`, the classes that have
   no compiled dictionary.

4. `Ghost` round-trips with RNTuple only because the interpreter parsed the same header the program
   was compiled with. Nothing guarantees that in a real job (layout, streamers, schema evolution all
   come from a compiled dictionary), so a framework should require the dictionary rather than rely
   on an interpreted class. The RField walk rejects it.

## Recommended policy for a framework writing data products

| Backend | Dictionaries required | Check |
|---|---|---|
| TTree | `T` **and** `std::vector<T>` | `HasDictionary()` on the top-level type; tell the user to declare `std::vector<T>` |
| RNTuple | user classes only (`T`, recursively) | walk the `RField` tree; report the class that lacks a dictionary |

Run the check when the container is set up for writing, and on the read path that every read goes
through, so a missing dictionary fails early with a clear message instead of later at read time.

## Files

| File | Role |
|---|---|
| `include/Shapes.h` | `demo::Point`, `demo::Pixel`, `demo::Ghost` (in a namespace: macOS `MacTypes.h` also defines `struct Point`, which ROOT's interpreter may have loaded) |
| `include/LinkDef.h` | dictionaries for `Point`, `std::vector<demo::Point>`, `Pixel` only |
| `src/probe.cpp` | the checks and the TTree / RNTuple round trips; also defines `Secret` |
| `scripts/run_all.sh` | runs every case in its own process |
| `env.sh` | lets an interactive `root` session find `libShapes` and `Shapes.h` |
