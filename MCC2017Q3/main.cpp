#include <iostream>
#include <string>
#include <vector>

int main() {
  int n;
  std::cin >> n;
  std::vector<char> balls{};
  for ( int a = 0; a < n; a++ ) {
    char b;
    std::cin >> b;
    balls.push_back( b );
  }
  bool check{true}; // Was there a collision previously? Default is always yes.
  int collisions{};
  while ( check ) { // Keep checking as long as there is collisions
    check = false;
    for ( int a = 0; a < n; a++ ) {
      if ( balls[ a ] == 'R' && a != n - 1 ) { // We only need to check the balls rolling right.
        if ( balls[ a + 1 ] == 'L' ) {         // Oh shit, there a ball rolling left!!

          // BONK!!
          balls[ a ] = 'L';
          balls[ a + 1 ] = 'R';

          // An accident has occured
          check = true;

          // Deputy MCC, add that into the counter.
          collisions++;
          break;
        }
      }
    }
  }
  std::cout << collisions << std::endl; // Report the crash count to the higher ups
}
