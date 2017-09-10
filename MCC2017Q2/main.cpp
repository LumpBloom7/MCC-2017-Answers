#include <iostream>
#include <string>
#include <vector>

int main() {
  int n;
  std::cin >> n; // Input n
  std::vector<std::string> words{};
  for ( int a = 0; a < n; a++ ) { // Input the words.
    std::string b;
    std::cin >> b;
    words.push_back( b );
  }
  int good{};
  for ( int a = 0; a < n; a++ ) {       // Scan through the vector of strings
    if ( words[ a ].size() % 2 != 0 ) { // First check if words are an odd length.
      int vowels = 0;
      for ( int b = 0; b < words[ a ].size(); b++ ) { // Scan through each character in the string
        if ( ( words[ a ][ b ] == 'a' || words[ a ][ b ] == 'e' || words[ a ][ b ] == 'i' || words[ a ][ b ] == 'o' ||
               words[ a ][ b ] == 'u' ) ) {
          vowels++; // +1 to the counter for every vowel found.
          if ( vowels >= 3 ) {
            good++;
            break;
          }
        }
      }
    }
  }
  std::cout << good << std::endl;
}
