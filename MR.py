from random import seed, randint

def decompose(N):
    ''' Decompose N into the form m * 2 ^ k.
        :returns: (m, k).
    '''
    m, k = N, 0
    while m & 1 == 0:
        m, k = m >> 1, k + 1
    return m, k


def MR(N, t=10):
    ''' Miller-Rabin test implementation.
        :param N: The integer to be tested.
        :param t: Number of loops when testing.
        :returns: Whether N is (most likely) a prime.
    '''
    if N < 5 or not N & 1:
        return N in (2, 3)
    m, k = decompose(N - 1)
    for i in range(t):
        a = randint(2, N - 2)
        y = pow(a, m, mod=N)
        if y not in (1, N - 1):
            for j in range(k - 1):
                y = pow(y, 2, mod=N)
                if y == 1:
                    return False
                if y == N - 1:
                    break
            if y != N - 1:
                return False
    return True


# Read the integers
with open("data/N1", "rb") as f:
    N1 = int.from_bytes(f.read(), "big")
with open("data/N2", "rb") as f:
    N2 = int.from_bytes(f.read(), "big")
# print(hex(N1))
# print(hex(N2))


# Apply M-R test
seed(2018011368) # reproducibility
print("M-R prime test result for N1:", MR(N1))
print("M-R prime test result for N2:", MR(N2))


# There are 9592 primes in [1, 100000]
print('[t = 1] "prime" count in 100000:', sum(MR(i, 1) for i in range(100000)))
print('[t = 2] "prime" count in 100000:', sum(MR(i, 2) for i in range(100000)))
print('[t = 3] "prime" count in 100000:', sum(MR(i, 3) for i in range(100000)))
