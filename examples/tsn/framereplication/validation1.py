import numpy as np
import itertools

combinations = np.array(list(itertools.product([0, 1], repeat=7)))
probabilities = np.array([0.8, 0.8, 0.64, 0.8, 0.64, 0.8, 0.8])
solutions = np.array([[1, 1, 1, 0, 0, 0, 0], [1, 1, 0, 0, 1, 1, 0], [1, 0, 0, 1, 1, 0, 0,], [1, 0, 1, 1, 0, 0, 1]])

p = 0
for combination in combinations:
    probability = (combination * probabilities + (1 - combination) * (1 - probabilities)).prod()
    for solution in solutions:
        if (solution * combination == solution).all() :
            p += probability
            break

print(p)

# result is 0.6566
