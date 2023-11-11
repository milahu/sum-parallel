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

## see also

- [Shell command to sum integers, one per line?](https://stackoverflow.com/questions/450799/shell-command-to-sum-integers-one-per-line)
- [Add up a column of numbers at the Unix shell](https://stackoverflow.com/questions/926069/add-up-a-column-of-numbers-at-the-unix-shell)
