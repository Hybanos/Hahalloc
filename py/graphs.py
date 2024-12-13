import matplotlib.pyplot as plt
import numpy as np
import sys
import os

if not os.path.exists("py/img"):
    os.mkdir("py/img")

def single_alloc():
    data = np.fromfile("py/single_alloc.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data)//3), 3).T

    y = (1 << 15) / y * 1e6
    z = (1 << 15) / z * 1e6

    plt.plot(x, y, label="hahalloc")
    plt.plot(x, z, label="malloc")
    plt.xscale('log', base=2)
    plt.yscale('log', base=10)
    # plt.show()
    plt.title("2^15 alloc/free of increasing size.")
    plt.legend()
    plt.xlabel("Allocation size")
    plt.ylabel("Allocs / seconds")
    plt.savefig("py/img/single_alloc.png")
    plt.close()

def calloc():
    data = np.fromfile("py/single_calloc.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data) // 3), 3).T

    y = (64) / y * 1e6
    z = (64) / z * 1e6

    plt.plot(x, y, label="chahalloc")
    plt.plot(x, z, label="calloc")
    plt.xscale('log', base=2)
    plt.yscale('log', base=10)
    # plt.show()
    plt.title("64 calloc/free of increasing size.")
    plt.legend()
    plt.xlabel("Allocation size")
    plt.ylabel("Allocs / seconds")
    plt.savefig("py/img/single_calloc.png")
    plt.close()

def multiple_alloc_fixed_size():
    data = np.fromfile("py/multiple_alloc_fixed_size.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data) // 3), 3).T

    x = x / 2
    y = (256) / y * 1e6
    z = (256) / z * 1e6

    plt.plot(x, y, label="hahalloc")
    plt.plot(x, z, label="malloc")
    # plt.show()
    plt.title("Active pointers fixed size")
    plt.legend()
    plt.xlabel("active pointers average count")
    plt.ylabel("Allocs / seconds")
    plt.savefig("py/img/multiple_alloc_fixed_size.png")
    plt.close()

def multiple_alloc_random_size():
    data = np.fromfile("py/multiple_alloc_random_size.txt", dtype="l", sep=" ")
    x, y, z = data.reshape((len(data) // 3), 3).T

    x = x / 2
    y = (256) / y * 1e6
    z = (256) / z * 1e6

    plt.plot(x, y, label="hahalloc")
    plt.plot(x, z, label="malloc")
    # plt.show()
    plt.title("Active pointers random size")
    plt.legend()
    plt.xlabel("active pointers average count")
    plt.ylabel("Allocs / seconds")
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

if __name__ == "__main__":
    single_alloc()
    multiple_alloc_fixed_size()
    multiple_alloc_random_size()
    realloc()
    calloc()