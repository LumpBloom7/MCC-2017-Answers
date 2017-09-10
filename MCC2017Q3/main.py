n = 10
balls = list(map(str,input().split()))

collisions = True
count = 0
while(collisions == True):
  collisions = False
  for x in range(balls-1):
    if(balls[ x ] == "R" and balls[ x+1 ] == "L"):
      balls[ x ] = "L"
      balls[ x + 1 ] = "R"
      count+=1
      collisions = True
      break

print(count)
