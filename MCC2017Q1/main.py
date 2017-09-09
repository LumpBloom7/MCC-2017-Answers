n = input()
eclipse = n + 1
if ( n % 3 == 0 ) : eclipse += 1
if ( ( n + 1 ) % 5 == 0 ) : eclipse += 2
if ( ( n + 2 ) % 7 == 0 ) : eclipse += 4
if ( ( n + 3 ) % 11 == 0 ): eclipse += 8
if ( eclipse == n + 1 ) : eclipse += 16
print( eclipse )
