/* This code doesn't work with the large input provided by MCC 2017
 * Refer to the Python code as that one works perfectly
 */
#include <iostream>

int main() {
  long long n;
  std::cin >> n;
  long long eclipse = n + 1;
  if ( n % 3 == 0 ) { eclipse++; }
  if ( ( n + 1 ) % 5 == 0 ) { eclipse += 2; }
  if ( ( n + 2 ) % 7 == 0 ) { eclipse += 4; }
  if ( ( n + 3 ) % 11 == 0 ) { eclipse += 8; }
  if ( eclipse == n + 1 ) { eclipse += 16; }
  std::cout << eclipse << std::endl;
}
