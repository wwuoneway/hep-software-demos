// Writes Event objects to a TTree. Works only if a dictionary for Event is available.
#include "Event.h"
#include "TFile.h"
#include "TTree.h"
#include <iostream>

int main(int argc, char** argv) {
  const char* out = argc > 1 ? argv[1] : "events.root";
  TFile f(out, "RECREATE");
  TTree t("events", "events");
  Event evt;
  t.Branch("event", &evt);  // needs the dictionary to introspect Event

  for (int i = 0; i < 5; ++i) {
    evt.run = 1;
    evt.number = i;
    evt.particles.clear();
    for (int j = 0; j <= i; ++j) evt.particles.push_back({0.f, 0.f, 1.f * j, 10.f * (j + 1), 0.f, 5.f, 11});
    t.Fill();
  }
  t.Write();
  std::cout << "entries written: " << t.GetEntries() << "\n";
  return 0;
}
