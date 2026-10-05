#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>


#define FILE "testfile.txt"

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


