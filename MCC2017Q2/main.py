n = int(input())
words=list(map(str,input().split()))

good = 0
for string in words:
  for letter in string:
    vowels = 0
    if letter == 'a' or letter == 'e' or letter == 'i' or letter == 'o' or letter == 'u':
      vowels += 1
      if(vowels = 3):
        good += 1
        break

print(good)
