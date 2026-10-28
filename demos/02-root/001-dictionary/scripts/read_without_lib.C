// Reads events.root WITHOUT loading libEvent: ROOT falls back to the StreamerInfo stored in the
// file (emulated class: data members only, no methods).
void read_without_lib(const char* fname = "build/events.root") {
  auto f = TFile::Open(fname);
  auto t = f->Get<TTree>("events");
  t->Print();
  t->Scan("event.run:event.number:event.particles.px");
}
