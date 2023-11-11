# sum-parallel

summarize numbers from a text file using multiple CPU cores

## status

the current [sum-parallel.c](sum-parallel.c) is slower than [sum.c](sum.c)

```
seq 10000000 >input.txt
gcc -o sum sum.c && time ./sum <input.txt
gcc -o sum-parallel sum-parallel.c && time ./sum-parallel input.txt
```

```
50000005000000

real    0m4.611s
user    0m4.553s
sys     0m0.058s
50000005000000

real    0m6.754s
user    0m12.317s
sys     0m2.780s
```

## cpu-bound or io-bound

[What do the terms "CPU bound" and "I/O bound" mean?](https://stackoverflow.com/questions/868568/what-do-the-terms-cpu-bound-and-i-o-bound-mean)

> adding two numbers takes a single CPU cycle, memory reads take about [100 CPU cycles](http://www.eecs.berkeley.edu/~rcs/research/interactive_latency.html) in 2016 hardware.

```console
$ pv -a input.txt >/dev/null
[1.52GiB/s]

$ pv -a input.txt | ./sum
[15.4MiB/s]
50000005000000
```

## read multiple numbers with fscanf

currently, i read only one number

```c
fscanf(fptr, "%d", &n)
```

but fscanf can read multiple values

```c
fscanf(fptr, "%d\n%d\n%d\n%d", &n1, &n2, &n3, &n4)
```

but no, fscanf is already buffered

https://stackoverflow.com/a/9587245/10440128

> When you use fread or the other file I/O functions in the C standard library, memory is buffered in several places.

> The C library will usually create a buffer for every FILE* you have open. Data is read into this buffers in large chunks. This allows fread to satisfy many small requests without having to make a large number of system calls, which are expensive. This is what people mean when they say fread is buffered.

> The kernel will also buffer files that are being read in the disk cache. This reduces the time needed for the read system call, since if data is already in memory, your program won't have to wait while the kernel fetches it from the disk. The kernel will hold on to recently read files, and it may read ahead for files which are being accessed sequentially.

## run multiple sum in parallel

todo

this runs serial

```
for i in $(seq 4); do seq $((1 + 10000000 / 4 * (i - 1))) $((10000000 / 4 * i)) | ./sum; done | ./sum
```

## see also

- [Shell command to sum integers, one per line?](https://stackoverflow.com/questions/450799/shell-command-to-sum-integers-one-per-line)
- [Add up a column of numbers at the Unix shell](https://stackoverflow.com/questions/926069/add-up-a-column-of-numbers-at-the-unix-shell)
