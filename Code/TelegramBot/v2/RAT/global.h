//===================================
// This file stores constants and methods that can be used in any other file. 
//===================================

#define LOCATION 255

int mod(int x, int n) {
  // Computes the module n over x
  return (x < 0) ? ((x % n) + n) : (x % n);
}

int sign(int x) {
  //Returns the sign of a number (1, -1)
  return abs(x)/x;
}

