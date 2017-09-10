N = 14
hats = [0, 1, 2, 2, 3, 4, 5, 6, 6, 6, 6, 6, 6, 6]
#Replace the above code with your test cases

liar=0

for i in range(2,N-1):
  if(hats[0]!=0):
    liar=1
  elif(hats[1]!=1):
    liar=2
  else:
    if(hats[i]+1!=hats[i+1] and hats[i]!=hats[i+1]):
      liar=i+1

if(liar==0):
  for x in range(len(hats)-1):
    if(hats[x]==hats[x+1]):
      liar=x+1
      break
print(liar)
