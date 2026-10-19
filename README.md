# TP3: Réalisation d’un Système de Fichiers simple

ENSEA 3rd year (IS), Systèmes et Réseaux.

## Build

```bash
make
```

Each `src/*.c` builds into `bin/<name>`, one file per question. The programs work on a copy of the test image, so the original stays untouched:

```bash
cp data/test_tosfs_files data/futosfs.img
mkdir -p /tmp/test_fuse
```

From question 2 on, a program mounts the file system with `./bin/<name> /tmp/test_fuse &` and `fusermount -u /tmp/test_fuse` unmounts it.

## Question 1: load the image with mmap

![Question 1](screenshots/chargement_mmap_1.png)

## Question 2: mount with FUSE

The operations table is still empty, so every request gets "Function not implemented".

![Question 2](screenshots/montage_fuse_2.png)

## Question 3: getattr

![Question 3](screenshots/getattr_3.png)

## Question 4: readdir

`ls` gets the entries, but not the attributes of the files yet: that needs `lookup`.

![Question 4](screenshots/readdir_4.png)

## Question 5: lookup

![Question 5](screenshots/lookup_5.png)

## Question 6: read

![Question 6](screenshots/read_6.png)

## Question 7: create and dir_add

After creating the file, `touch` sets its times. tosfs inodes have no times, so `setattr` just answers with the current attributes.

![Question 7](screenshots/create_7.png)

## Question 8: write

`setattr` now also handles the truncation done by `>`. A file holds at most one block of 4096 bytes.

![Question 8](screenshots/write_8.png)
