#include <stdio.h>
#include "../segment.hpp"

auto r1 = Ramp(10, 0, 1);
auto signal1 = rev(r1);
auto signal = repeat(cat(r1, rev(r1)), 3);

int main() {
  printf("d %f v %f \n", signal.duration_ms(), signal.data(10));
  
  for (int i = 0; i < signal.duration_ms(); i++) {
    printf("%03d : %f, \n", i, signal.data(i));
  }
  
}
