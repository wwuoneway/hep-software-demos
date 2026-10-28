// Loads libEvent so the real C++ classes are available.
#include "Event.h"
void read_with_lib(const char* fname = "build/events.root") {
  gSystem->Load("build/libEvent");
  auto f = TFile::Open(fname);
  auto t = f->Get<TTree>("events");
  Event* evt = nullptr;
  t->SetBranchAddress("event", &evt);
  for (Long64_t i = 0; i < t->GetEntries(); ++i) {
    t->GetEntry(i);
    printf("event %d: %zu particles, first px=%.1f\n", evt->number, evt->particles.size(), evt->particles[0].px);
  }
}
