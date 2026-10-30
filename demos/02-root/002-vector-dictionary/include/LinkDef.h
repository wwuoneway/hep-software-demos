#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// Point: the class AND std::vector<Point> get a dictionary.
#pragma link C++ class demo::Point+;
#pragma link C++ class std::vector<demo::Point>+;

// Pixel: only the class. std::vector<Pixel> is deliberately NOT listed.
#pragma link C++ class demo::Pixel+;

// Ghost: nothing. Neither Ghost nor std::vector<Ghost> gets a dictionary.
#endif
