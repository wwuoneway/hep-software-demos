#pragma once

// A namespace keeps the names clear of system types: macOS MacTypes.h also defines `struct Point`,
// and ROOT's interpreter may have seen it.
namespace demo {

  // Plain structs. LinkDef.h decides which ones get a dictionary.
  struct Point {
    float x = 0, y = 0;
    bool operator==(Point const&) const = default;
  };

  struct Pixel {
    int row = 0, col = 0;
    bool operator==(Pixel const&) const = default;
  };

  // Ghost: no dictionary is generated (LinkDef.h leaves it out), but this header is visible to
  // ROOT's interpreter, so ROOT can still build an *interpreted* TClass for it.
  struct Ghost {
    int id = 0;
    double energy = 0;
    bool operator==(Ghost const&) const = default;
  };

} // namespace demo
