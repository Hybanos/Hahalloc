import matplotlib.pyplot as plt
import numpy as np
import sys
import os

if not os.path.exists("py/img"):
    os.mkdir("py/img")

def single_alloc():
    data = np.fromfile("py/single_alloc.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data)//3), 3).T

    plt.plot(x, y)
    plt.plot(x, z)
    plt.xscale('log', base=2)
    plt.yscale('log', base=2)
    # plt.show()
    plt.savefig("py/img/single_alloc.png")
    plt.close()

def multiple_alloc_fixed_size():
    data = np.fromfile("py/multiple_alloc_fixed_size.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data) // 3), 3).T

    plt.plot(x, y)
    plt.plot(x, z)
    # plt.show()
    plt.savefig("py/img/multiple_alloc_fixed_size.png")
    plt.close()

def multiple_alloc_random_size():
    data = np.fromfile("py/multiple_alloc_random_size.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data) // 3), 3).T

    plt.plot(x, y)
    plt.plot(x, z)
    # plt.show()
    plt.savefig("py/img/multiple_alloc_random_size.png")
    plt.close()

def realloc():
    data = np.fromfile("py/re_alloc.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data) // 3), 3).T

    plt.plot(x, y)
    plt.plot(x, z)
    # plt.show()
    plt.savefig("py/img/re_alloc.png")
    plt.close()

def calloc():
    data = np.fromfile("py/single_calloc.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data)//3), 3).T

    plt.plot(x, y)
    plt.plot(x, z)
    plt.xscale('log', base=2)
    plt.yscale('log', base=2)
    # plt.show()
    plt.savefig("py/img/single_calloc.png")
    plt.close()


if __name__ == "__main__":
    single_alloc()
    multiple_alloc_fixed_size()
    multiple_alloc_random_size()
    realloc()
    calloc()