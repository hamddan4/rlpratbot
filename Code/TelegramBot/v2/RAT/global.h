//===================================
// This file stores constants and methods that can be used in any other file. 
//===================================

#define LOCATION 255

int mod(int x, int n) {
  return (x < 0) ? ((x % n) + n) : (x % n);
}

int sign(int x) {
  return abs(x)/x;
}

