/* ASCII numbers start at 48
 * So the char '0' would correspond to the number 48
 * If we deduct 48 from the ints converted from chars
 * We could easily get the correct conversion
 */

#include <iostream>
#include <string>
#include <vector>

// Prepare variables.
char prefix, suffix;
std::string number;
std::vector<int> n{};

void solve( int index, std::string curr ) {
  std::string temp = curr;
  for ( int a = 48; a < 54; a++ ) {
    temp[ n[ index ] ] = char( a );
    if ( index == n.size() - 1 ) { // If we're at the last '?', start counting the sum.
      int a = int( temp[ 0 ] ) - 48;
      int b = int( temp[ 1 ] ) - 48;
      int c = int( temp[ 2 ] ) - 48;
      int d = int( temp[ 3 ] ) - 48;
      int sum = a + b + c + d;
      if ( suffix == 'A' ) { // We know the suffix is 'A', so if our sum is uneven. We ignore it.
        if ( sum % 2 == 0 ) { std::cout << prefix << ' ' << temp << ' ' << suffix << std::endl; }
      } else if ( suffix == 'B' ) { // We know the suffix is 'B', so if our sum is even.We ignore it.
        if ( sum % 2 != 0 ) { std::cout << prefix << ' ' << temp << ' ' << suffix << std::endl; }
      } else { // We don't n=know the suffix, so anything will do.

        char tsuffix; // But we need to use a temporary suffix, because otherwise the recursion will fail to properly
                      // solve
        if ( sum % 2 != 0 ) {
          tsuffix = 'B';
        } else {
          tsuffix = 'A';
        }
        std::cout << prefix << ' ' << temp << ' ' << tsuffix << std::endl;
      }
    } else { // Not the last '?', so keep working.
      solve( index + 1, temp );
    }
  }
}
int main() {
  while ( true ) {
    n = {};
    std::cin >> prefix >> number >> suffix;

    prefix = 'S'; // Prefix is alway 'S' no matter what. So always replace prefix with 'S' for simplicity

    for ( int a = 0; a < 4; a++ ) {
      if ( number[ a ] == '?' ) { n.push_back( a ); } // Get the index of '?' present in number.
    }

    if ( n.size() == 0 ) { // Wait... There is no '?' in the number.
      int a = int( number[ 0 ] ) - 48;
      int b = int( number[ 1 ] ) - 48;
      int c = int( number[ 2 ] ) - 48;
      int d = int( number[ 3 ] ) - 48;

      if ( ( a + b + c + d ) % 2 != 0 ) { // Since all four numbers exist, we could findthe checksum easily.
        suffix = 'B';
      } else {
        suffix = 'A';
      }
      std::cout << prefix << ' ' << number << ' ' << suffix << ' ' << std::endl;
    } else { // Looks like we got some '?'s. Boys, let's get working.
      solve( 0, number );
    }
  }
}
