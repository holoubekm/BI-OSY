# BI-OSY Course
## Operační systémy
### CTU / ČVUT FIT

The main course consists of two semestral works - each focused on a slightly different topic.

### Semestral work 1
Implementation of classic producer-consumer task. There were two types of tasks, each consuming different resources. Successful solution had to be dynamic in terms of tasks assignment and memory consumption. Furthermore both tasks could be parallelised even more - they involve matrix calculations. Thus it was possible to split these computations into finer grained subtasks.

### Semestral work 2
The second semestral work was to write a simple memory allocator. Four functions we had to allocate were: `HeapInit`, `HeapDone`, `HeapAlloc`, `HeapFree` given a raw block of memory. We weren't enforced to use a specific algorithm, but using bucket algorithm and splitting a block into differently sized sublocks was fairly reasonable.
