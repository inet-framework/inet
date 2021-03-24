import random

successful = 0
n = 1000000

for i in range(n):
    s1 = random.random() < 0.8
    s1_s2a = random.random() < 0.8
    s1_s2b = random.random() < 0.8
    s2a_s2b = random.random() < 0.8
    s2b_s2a = random.random() < 0.8
    s2a = (s1 and s1_s2a) or (s1 and s1_s2b and s2b_s2a)
    s2b = (s1 and s1_s2b) or (s1 and s1_s2a and s2a_s2b)
    s3a = s2a and (random.random() < 0.8)
    s3b = s2b and (random.random() < 0.8)
    s3a_s4 = random.random() < 0.8
    s3b_s4 = random.random() < 0.8
    s4 = (s3a and s3a_s4) or (s3b and s3b_s4)
    if s4:
        successful += 1

print(successful / n)

# result is 0.657
