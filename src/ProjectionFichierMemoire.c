#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


#define FILE "testfile.txt"


//struct stat { //Copié depuis stat(3type) — Linux manual page
//           dev_t      st_dev;      // ID of device containing file */
//           ino_t      st_ino;      // Inode number */
//           mode_t     st_mode;     /* File type and mode */
//           nlink_t    st_nlink;    /* Number of hard links */
//           uid_t      st_uid;      /* User ID of owner */
//           gid_t      st_gid;      /* Group ID of owner */
//           dev_t      st_rdev;     /* Device ID (if special file) */
//           off_t      st_size;     /* Total size, in bytes */
//           blksize_t  st_blksize;  /* Block size for filesystem I/O */
//           blkcnt_t   st_blocks;   /* Number of 512 B blocks allocated */

           /* Since POSIX.1-2008, this structure supports nanosecond
              precision for the following timestamp fields.
              For the details before POSIX.1-2008, see HISTORY.  */

//           struct timespec  st_atim;  /* Time of last access */
//           struct timespec  st_mtim;  /* Time of last modification */
//           struct timespec  st_ctim;  /* Time of last status change */

//       #define st_atime  st_atim.tv_sec  /* Backward compatibility */
//       #define st_mtime  st_mtim.tv_sec
//       #define st_ctime  st_ctim.tv_sec
//       }; 


void inverseBytes(char *addr, long int size){
    char startFileByte;
    char endFileByte;
    for(int i=0; i<=(int)size/2;i++){
        startFileByte = addr[i];
        endFileByte = addr[size - i - 1];
        addr[i] = endFileByte;
        addr[size - i - 1] = startFileByte;
    }
}


int main(){
    //ouverture fichier avec open
    int file_descriptor = open(FILE, O_RDWR | O_CREAT, S_IRWXU);
    if(file_descriptor==-1){ 
        perror("open");     //Affiche l'erreur en commençant par le mot-clé open
        exit(EXIT_FAILURE);
    }
    //lecture taille fichier avec stat
    struct stat statbuff;
    int return_value_stat = stat(FILE, &statbuff);
    if(return_value_stat!=0){
        perror("stat");
        exit(EXIT_FAILURE);
    }
    printf("La taille du fichier est de %ld octets\r\n",statbuff.st_size);  //off_t st_size; -> off_t est un entier signé

    //mapping du fichier avec mmap
    char *addr;
    addr = mmap(NULL, statbuff.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (addr == MAP_FAILED){
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    //inversement des octets du FILE
    inverseBytes(addr,statbuff.st_size);


    //démapping avec munmap
    int return_value_munmap = munmap(addr, statbuff.st_size);
    if(return_value_munmap !=0 ){
        perror("munmap");
        exit(EXIT_FAILURE);  
    }
    //passage à l'exec cat testfile.txt
    execlp("cat","cat",FILE,NULL);
    perror("execlp");
    exit(EXIT_FAILURE);

}


