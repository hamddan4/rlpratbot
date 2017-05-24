
int mod(int x, int n) {
  return (x < 0) ? ((x % n) + n) : (x % n);
}

int sign(int x) {
  return abs(x)/x;
}

