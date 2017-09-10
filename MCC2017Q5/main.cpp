#include <iostream>
#include <string>
#include <vector>

#include <algorithm>
#include <iterator>
#include <functional>

int main() {
  int n;
  std::cin >> n;
  std::vector<int> hats{};

  for ( int a = 0; a < n; a++ ) {
    int b;
    std::cin >> b;
    hats.push_back( b );
  }
  int Jimmy = 0;

  for ( int a = 0; a < n; a++ ) {
    if ( hats[ 0 ] != 0 ) {
      Jimmy = 1;
      break;
    } else if ( hats[ 1 ] != 1 ) {
      Jimmy = 2;
      break;
    } else {
      if ( hats[ a ] + 1 != hats[ a + 1 ] && hats[ a ] != hats[ a + 1 ] ) {
        Jimmy = a + 1;
        break;
      }
    }
  }
  if ( Jimmy == 0 ) {
    for ( int a = 0; a < n; a++ ) {
      if ( hats[ a ] == hat[ a + 1 ] ) {
        Jimmy = a + 1;
        break;
      }
    }
  }
  std::cout << Jimmy << std::endl;
}
