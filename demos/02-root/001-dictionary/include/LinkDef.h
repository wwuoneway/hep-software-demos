#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class Particle+;
#pragma link C++ class Event+;
// Template instantiations used as members need their own entry.
#pragma link C++ class std::vector<Particle>+;
#endif
