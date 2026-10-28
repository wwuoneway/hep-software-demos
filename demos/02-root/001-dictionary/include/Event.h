#pragma once
#include <vector>

struct Particle {
  float x = 0, y = 0, z = 0;
  float px = 0, py = 0, pz = 0;
  int pdgId = 0;
};

struct Event {
  int run = 0;
  int number = 0;
  std::vector<Particle> particles;
};
