# TP1: memory segmentation and linked lists

ENSEA 3rd year (IS), Systèmes et Réseaux.

## Build

```bash
make
```

Each `src/*.c` builds into `bin/<name>`. Run the programs from this folder: exercise 2 reverses `data/test.txt` in place, so a second run restores it.

| Exercise | Source | Run |
|---|---|---|
| 1. Memory segments, checked with `pmap -X` | `src/ex1_memory_segments.c` | `./bin/ex1_memory_segments` |
| 2. File mapping with `mmap` | `src/ex2_file_mapping.c` | `./bin/ex2_file_mapping` |
| 3. Linked lists, one file per question | `src/ex3_q01_create.c` to `src/ex3_q11_circular.c` | `./bin/ex3_q01_create` to `./bin/ex3_q11_circular` |

## Screenshots

Exercise 1

![Exercise 1](screenshots/ex1_memory_segments.png)

Exercise 2

![Exercise 2](screenshots/ex2_file_mapping.png)

Exercise 3

| Question | Screenshot |
|---|---|
| 1. Create the list | ![Q1](screenshots/ex3_q01_create.png) |
| 2. Length | ![Q2](screenshots/ex3_q02_length.png) |
| 3. Print | ![Q3](screenshots/ex3_q03_print.png) |
| 4. Remove the first element | ![Q4](screenshots/ex3_q04_remove_first.png) |
| 5. Remove the last element | ![Q5](screenshots/ex3_q05_remove_last.png) |
| 6. Append | ![Q6](screenshots/ex3_q06_append.png) |
| 7. Prepend | ![Q7](screenshots/ex3_q07_prepend.png) |
| 8. Concatenate | ![Q8](screenshots/ex3_q08_concat.png) |
| 9. Map | ![Q9](screenshots/ex3_q09_map.png) |
| 10. Doubly linked list | ![Q10](screenshots/ex3_q10_doubly_linked.png) |
| 11. Circular doubly linked list | ![Q11](screenshots/ex3_q11_circular.png) |
