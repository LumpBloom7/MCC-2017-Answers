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

  // Find number of duplicates in a row.
  std::vector<int> dupe{};
  auto i = begin( hats );
  while ( i != end( hats ) ) {
    auto ub = adjacent_find( i, end( hats ), std::not_equal_to<int>() );
    if ( ub == end( hats ) ) {
      dupe.push_back( distance( i, ub ) );
      break;
    }
    dupe.push_back( distance( i, ub ) + 1 );
    i = next( ub );
  }

  int Jimmy = 0;
  int change = 0; // Keep track of how many color changes there where
  int last = 0;   // Get the number of the previous person.

  // A lot of stuff that I'm to lazy to explain.
  for ( int a = 0; a < n; a++ ) {
    if ( hats[ a ] != 0 && a == 0 ) {
      Jimmy = 1;
      break;
    } else if ( hats[ a ] != 1 && a == 1 ) {
      Jimmy = 2;
      break;
    } else if ( hats[ a ] != last ) {
      if ( hats[ a ] < last ) {
        if ( hats[ a ] < hats[ a + 1 ] ) {
          Jimmy = a + 1;
          break;
        }
        if ( dupe[ change ] == 1 ) {
          Jimmy = a + 1;
          break;
        } else if ( dupe[ change ] > 1 ) {
          Jimmy = a;
          break;
        }
      } else if ( hats[ a ] > last + 1 ) {
        if ( hats[ a ] > hats[ a + 1 ] ) {
          Jimmy = a + 1;
          break;
        }
        if ( dupe[ change ] == 1 ) {
          Jimmy = a + 1;
          break;
        } else if ( dupe[ change ] > 1 ) {
          Jimmy = a;
          break;
        }
      }
      last = hats[ a ];
      change++;
    }
  }
  if ( Jimmy == 0 ) {
    for ( int a = 2; a < n; a++ ) {
      if ( hats[ a ] != a ) {
        Jimmy = a;
        break;
      }
    }
    if ( Jimmy == 2 ) { Jimmy++; }
  }

  std::cout << Jimmy << std::endl;
}
