#include <iostream>
#include <string>
#include <vector>

#include <algorithm>
#include <iterator>
#include <functional>

int main() {
  int x, k;           // Prepare ints for x and k
  std::cin >> x >> k; // Receive input and put it into x and k

  std::vector<int> blue{}, red{}; // Prepare vector for both portals, naming doesn't matter.

  for ( int a = 0; a < k; a++ ) { // Input first set of portals
    int b;
    std::cin >> b;
    red.push_back( b );
  }
  for ( int a = 0; a < k; a++ ) { // Input second set of portals
    int b;
    std::cin >> b;
    blue.push_back( b );
  }
  // Sort both pairs of portals just in case.
  std::sort( blue.begin(), blue.end() );
  std::sort( red.begin(), red.end() );

  int playerPos = 0; // We know player starts at 0
  int steps = 0;     // Prepare counter for steps.

  while ( playerPos != x ) { // While player is not at x, keep looping.

    if ( playerPos < x ) { // Make the player walk right if he is on the left of x
      playerPos++;
      steps++;
    } else if ( playerPos > x ) { // Make the player walk left if he is on the right of x
      playerPos--;
      steps++;
    }

    // Check if player is on a red portal after moving.
    auto it = std::find( red.begin(), red.end(), playerPos );
    if ( it == red.end() ) {
      // Player is not standing on any red portals.
    } else {                                         // He is standing on a red portal
      auto index = std::distance( red.begin(), it ); // Get index of red portal
      if ( playerPos < x ) { // If Player is on the left of x, Select the furthest blue portal to the right
        if ( blue[ blue.size() - 1 ] > playerPos ) { // Check if blue[blue.size() -1 ] is on the right of player,
          // because if portal on the right of the player does not exists we
          // don't teleport

          playerPos = blue[ blue.size() - 1 ]; // Set player position to be the same as the blue portal
          // Remove both portals because each portal can only be used once.
          blue.erase( blue.end() - 1 );
          red.erase( red.begin() + index );
        }
      } else if ( playerPos > x ) {    // If Player is on the right of x, Select the furthest blue portal on the left
        if ( blue[ 0 ] < playerPos ) { // Check if blue[0] is on the left of player, because if portal on the left of
                                       // the player does not exists we don't teleport

          playerPos = blue[ 0 ]; // Set player position to be the same as the blue portal
          // Remove both portals because each portal can only be used once.
          blue.erase( blue.begin() );
          red.erase( red.begin() + index );
        }
      }
    }
    // Check if player is on a blue portal after moving.
    it = std::find( blue.begin(), blue.end(), playerPos );
    if ( it == blue.end() ) {
      // Player is not standing on any blue portals.
    } else {                                          // He is standing on a blue portal
      auto index = std::distance( blue.begin(), it ); // Get index of blue portal
      if ( playerPos < x ) { // If Player is on the left of x, Select the furthest red portal to the right
        if ( red[ red.size() - 1 ] > playerPos ) { // Check if red[red.size() -1 ] is on the right of player,
                                                   // because if portal on the right of the player does not exists we
                                                   // don't teleport

          playerPos = red[ red.size() - 1 ]; // Set player position to be the same as the red portal
          // Remove both portals because each portal can only be used once.
          red.erase( red.end() - 1 );
          blue.erase( blue.begin() + index );
        }
      } else if ( playerPos > x ) {   // If Player is on the right of x, Select the furthest red portal on the left
        if ( red[ 0 ] < playerPos ) { // Check if red[0] is on the left of player, because if portal on the left of
                                      // the player does not exists we don't teleport

          playerPos = red[ 0 ]; // Set player position to be the same as the red portal
          // Remove both portals because each portal can only be used once.
          red.erase( red.begin() );
          blue.erase( blue.begin() + index );
        }
      }
    }
  }
  std::cout << steps << std::endl; // Output number of steps
}
