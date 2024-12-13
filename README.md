# Hahalloc

A lightweight, thread-safe[*](#thread-safety) memory allocator written in C, that can sometimes be faster than malloc. It is designed to track memory leaks and gives you a heap summary at the end of your program's execution.

The API is the following:

| Hahalloc | STD equivalent | 
|---|--- |
| hahalloc | malloc |
| frhehe | free |
| chahalloc | calloc |
| rehehalloc | realloc |
| mem_lhihiks | |

### API reference

```C
void *hahalloc(size_t size);
```
Behaves the exact same way as `malloc`: returns a pointer of `size` bytes.

```C
void frhehe(void *ptr);
```
Same as `free`, liberates the pointer's memory. Further use of `ptr` might result in an immidiate crash, it should be set to `NULL`.

```C
void *rehehalloc(void *ptr, size_t size);
```
Once again this behaves like it's STD's counterpart, with the difference that `size` can be smaller than the allocated size of `ptr`. A pointer to the newly-allocated memory is returned.

```C
void *chahalloc(size_t size);
```
`calloc` equivalent. Note that the arguments differ as `chahalloc` only takes a `size` argument to be more fitting with `hahalloc`.

```C
void mem_lhihiks(size_t size);
```
You can call this function at any time to print a heap summary to `stdout`.

# Using Hahalloc
To compile the library, simply clone the repo and run `make`.
To execute tests and / or performance benchmark, run `make tests`, `make bench` or `make all`.
To link this library to another program, run it with: 
```bash
LD_LIBRARY_PATH=<path_to_libhahaloc.so>:$LD_LIBRARY_PATH
```

# Inner workings

This repo is an assignment for the first year of my master's degree in high performance computing. As i could have followed some tutorials on implementing memory allocators, i decided not to and went in pretty much blind. If some of the terminology or concepts explained here seem made up, that's why. 

### General definitions and explanation

The library is basically a big `mmap` and `munmap` wrapper.
Each call to `mmap` gives us a chunk of memory to work with, and each corresponding call to `munmap` releases it. Let's call these chunks **mappings**.

We could simply hand-off these mappings to the user but that would be pretty sub-optimal as there is a lot of overhead in asking memory to the OS. 
In fact, the mappings are over-allocated, and divided into parts that each act as an individual pointer sent to the user. Each part is called a **range**. Ranges are the base of the structure of Hahalloc.

### Data structure

In order to allow the allocation of *"infinite"* ranges, a double linked list is used. This list links every range to its neighboors (*"right"* and *"left"* neighboors as in directly before and after in memory) and across mappings.


Ranges and mappings can be represented as such:
```
├───────────────── mapping 1 ──────────────┤    ├───────────────── mapping 2 ──────────────┤ 
├───── range 1 ────┤    ├───── range 2 ────┤    ├───── range 3 ────┤    ├───── range 4 ────┤
┏━━━━━━┳━━━━━━━━━━━┓    ┏━━━━━━┳━━━━━━━━━━━┓    ┏━━━━━━┳━━━━━━━━━━━┓    ┏━━━━━━┳━━━━━━━━━━━┓
┃ meta ┃ user data ┃ -> ┃ meta ┃ user data ┃ -> ┃ meta ┃ user data ┃ -> ┃ meta ┃ user data ┃ ...
┗━━━━━━┻━━━━━━━━━━━┛    ┗━━━━━━┻━━━━━━━━━━━┛    ┗━━━━━━┻━━━━━━━━━━━┛    ┗━━━━━━┻━━━━━━━━━━━┛
```
With this structure, we just have to store a pointer to the first range on the stack and we can let it expand itself as the user requests more memory.
However, for the sake of optimisation, there are a total of 21 ranges stored on the stack.
This allows allocations of similar sizes to stay close to eachother, preventing some fragmentation of the memory.
Ranges that are pointed to from the heap are called **root ranges** and have a special flag in their metadata.

Note that no memory is mapped if the user doesn't ask for it. This means that the static array of pointers-to-root-ranges is initially an array of `NULL`.

We can represent the total memory structure like so:
```
┏━━━━━━━━━┓    ┏━━━━━━━━━━━┓    ┏━━━━━━━━━━━┓
┃ ROOT  1 ┃ -> ┃ mapping 1 ┃ -> ┃ mapping 2 ┃ ...
┣━━━━━━━━━┫    ┗━━━━━━━━━━━┛    ┗━━━━━━━━━━━┛
┃ ROOT  2 ┃ -> NULL
┗━━━━━━━━━┛    
   ...
┏━━━━━━━━━┓    ┏━━━━━━━━━━━┓    ┏━━━━━━━━━━━┓
┃ ROOT 20 ┃ -> ┃ mapping 1 ┃ -> ┃ mapping 2 ┃ ...
┣━━━━━━━━━┫    ┣━━━━━━━━━━━┫    ┗━━━━━━━━━━━┛
┃ ROOT 21 ┃ -> ┃ mapping 1 ┃ -> NULL
┗━━━━━━━━━┛    ┗━━━━━━━━━━━┛
```
Better justification of this choice of structure can be found in the [Optimisation](#optimisations) section.

### Range struct

Range metadata is made up of 4 components:
```C
typedef struct range {
    struct range *prev;
    struct range *next;
    size_t size;
    unsigned short meta;
} range;
```

`prev` and `next` are obviously pointers to linked neighboors,
`size` is the number of bytes of the range, or the memory sent to the user
`meta` is a byte containing 4 bit flags:
```
00000000
    │││└─ range is the root of a list
    ││└── range is allocated
    │└─── range is the start of a mapping
    └──── oversize mapping
```
This would, in theory, optimise the size of a range and save some space with lots of active pointers, but is in fact not necessary because GCC adds padding to structs, making them 8-bytes aligned.
Another way to see it is that we still have 7 bytes free for use in the struct.

# Optimisations

Here is a list of optimisations implemented:

### Mapping recycling
Mapping are allocated to **10x** the size of the biggest range that fits in them. 
For example: mappings on root number 10 are $2^{18}\times 10$ bytes long and store ranges that are between $2^{17}$ and $2^{18}$ bytes. 
This allows us to give ranges extremely fast when the mapping is not full.

### Root size groups
Grouping ranges of similar sizes together prevents us for saturating the linked lists and thus reduces the time to find a free range. 

### Memory de-fragmentation
Memory fragmentation is a huge issue, and even more so as the number of active pointers grow. Time to find a free range increases linearly with the number of ranges of a linked list, which can cause severe performance issue long-term.
Several mecanisms are in place to prevent memory fragmentation:

#### Coalescence
This is simply the act of merging neighbooring free ranges together. Not only does this reduces the size of our linked list, it also increases dramatically the chance of our next request fitting.
This merging of both left and right neighboors justifies the choice of using a double linked list and guarantees we never have 2 free blocks next to eachother (on the same mapping of course).  

#### Innacessible range prevention
This optimisation is a bit less obvious. We sometimes encounter a free range smaller than what could ever be allocated on a root.

For example: Allocating 10MB twice, freeing the first pointer, then allocating 8MB. This would result in 3 ranges: `[8MB allocated] - [2MB free] - [10MB allocated]`.
The 2MB middle range could never be allocated, as the minimun size on that root is 5MB. This creates an unnecessarry range and wastes memory.
To prevent this issue, the third allocation of 8MB will simply flip the 10MB free range and flag it as allocated before returning the pointer. 

### Oversize allocation
In order to prevent mappings from getting too big, requests above $2^{29}$ bytes are not assigned to any root, and are returned on their own. They are also not recycled and are `munmap`ed instantly once `frhehe` is called.


### Rehehalloc
Rehehalloc has a few optimisations of its own.

First, resizing to something smaller than before is enabled. When this is the case, we directly return the pointer with no modification.

Then, if the next range is free, it is expanded into, and once again prevents the need of `memcpy`.

If none of the above, we allocate a new range and `memcpy` the data just like `realloc` does.

# Thread safety
This library is thread-safe-**ish**. A mutex is locked before any write operation on the allocator's metadata. HOWEVER, this mutex seems to break if too many threads try to access it at once. For example, having $\le 4$ running allocs concurently doesn't cause any issue, but when going past 4, everything breaks by either deadlocking or core dumping.
This may or may not be related to my cpu's (4 cores - 8 threads) capabilities, but in any way it's a rabbithole I haven't had time to enter.

# Performance
The main metric when it comes to performance is allocations per seconds.

---

The 2 following benchmark are "brute force" tests. We simply allocate some memory, free it, and repeat.

Here we compare `hahalloc` and `malloc` by asking $2^{15}$ pointers of increasing sizes. 
![haha](py/img/single_alloc.png)

We can see that performace is quite even, with a few variations on `malloc`'s side probably due to some complex inner-workings. We can also clearly see the change of regime of both allocators when pointers get excessively big.


Pretty much the same thing, but for `chahalloc`/`calloc`
![haha](py/img/single_calloc.png)

Once again, performance is very similar. `calloc` seems to get a lot smarter past $2^{22}$ bytes tho.

---

This sections is dedicated to test `hahalloc` in a more """real life""" scenario.
These tests are allocating pointers and freeing them, but **in a random order**. Meaning that we keep a buffer of pointers and randomly pick when to free/allocate slots in this buffer.

Here, all allocations are 1MiB.
![haha](py/img/multiple_alloc_fixed_size.png)

For this test, `hahalloc` has a small advantage until the linked list gets a bit too large and `mallocs` catches up. 
Also wow i made waves ??

This is a very similar test as the previous one, except allocations are random sizes between 1 and 1Mi bytes.
![haha](py/img/multiple_alloc_random_size.png)

Here the multiple roots are clearly paying off, we are able to keep up with `malloc` until about 170 active pointers in average, and performance stays really close after that.

---

> [!NOTE]
> Benchmarks done here are heavily biased towards hahalloc, taking usage of the most important features of the allocator. malloc would probably crush these performances if benchmarks were biased towards it.

With that in mind, it's safe to say that in cherry-picked short-running applications and high-entropy pointer allocations, Hahalloc is faster than the standard library.

# Possible improvements

### Data structure
Many improvements could be done to the data structure. First, the usage of linked list is justified when switching between mappings, but gets harder to defend when working from range to range in the same mapping. Having a more static data structure inside of those would probably make a lot more sense and improve performances a lot.

Then, it would be interesting to store as litle data as possible on the stack. For example, only having a pointer to the first root and having everything else expand in the heap on its own. This is currently not done because it would require a new way of initializing the data structure with it's own `mmap` and `munmap`.

### Oversize mappings.
I don't really like the way oversize mappings are managed. Similarly, `mmap`ing 10x the size of requests close to a gigabyte and keeping them there until the program ends seems like a big limitation. It would be a big improvement to either change how these 2 regimes work or find a way to merge them into a more modular but consistent system.

### Thread safety
Improving thread safety would be a good plus, at least making it work for n threads and not limit it to 4. Then, leaving the mutex behind and switching to something really scalable.