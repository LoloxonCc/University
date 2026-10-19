import random
import math

def distance(x1, y1, x2, y2):
    return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

N = int(input("Podaj ilość iteracji: "))
K = int(input("Podaj krok: "))

pointsInsideCircle = 0
allPoints = 0

for i in range(N):
    x = random.uniform(-1, 1)
    y = random.uniform(-1, 1)
    if distance(0, 0, x, y) <= 1:
        pointsInsideCircle += 1
    allPoints += 1

    if allPoints % (N / K) == 0:
        print("Oszacowanie pi, po", allPoints, "krokach:",4 * (pointsInsideCircle / allPoints))

print("Różnica między wartością math.pi, a wyliczoną:", abs(math.pi - 4 * (pointsInsideCircle / allPoints)))
