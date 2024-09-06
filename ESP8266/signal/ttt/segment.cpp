#include <stdio.h>
#include "../segment.hpp"

auto r = Ramp(10, 0, 1);
auto signal1 = rev(r);
//auto signal = repeat(cat(r, rev(r)), 3);

int main() {
  auto r = Ramp(10, 0, 1);
  auto signal1 = rev(r);
  auto signal = repeat(cat(r, rev(r)), 3);
  for (int i = 0; i < signal.duration_ms(); i++) {
    printf("%f, ", signal.data(i));
  }
  
}
