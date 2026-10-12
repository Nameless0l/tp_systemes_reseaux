#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BIRTHYEAR_FADE 2004
#define BYTES_FOR_BIRTHYEAR_FADE 4*sizeof(int)
#define NAME_COLLEGUE "MBASSI"
#define BYTES_FOR_NAME_COLLEGUE 6*sizeof(int)
#define CURRENT_MONTH 12L
#define BYTES_FOR_CURRENT_MONTH 2*sizeof(long)


void *malloc_3is(size_t size){
    return sbrk(size); //L'erreur sera gérée dans le main
}

//void free_3is(void *_Nullable p){}



int main(){ //Tests sur notre malloc_3is et free_3is
    //On doit en premier lieu récupérer l'adresse du heap initialisé du process
    //se qui se fait par appeler srbk(0)
    void * allocated_memory_start_ptr = sbrk(0);
    if(allocated_memory_start_ptr == (void *) -1){
        perror("srbk(0) : ");
        exit(EXIT_FAILURE);
    }
    printf("Our starting pointer is at : %p\r\n",allocated_memory_start_ptr);
    
    //1ère allocation : INT
    int * number_allocated = malloc_3is(BYTES_FOR_BIRTHYEAR_FADE);
    *number_allocated = BIRTHYEAR_FADE;
    if(number_allocated == (void *) -1){
        perror("malloc_3is : ");
        exit(EXIT_FAILURE);
    }
    printf("FADE birthyear is %d\r\n",*number_allocated);
    printf("The respective brk addr is : %p\r\n",number_allocated);



    //2nd allocation : CHAR *
    char ** string_allocated = malloc_3is(BYTES_FOR_NAME_COLLEGUE); 
    *string_allocated = NAME_COLLEGUE;
        if(string_allocated == (void *) -1){
        perror("malloc_3is : ");
        exit(EXIT_FAILURE);
    }
    printf("Collegue's name is %s\r\n",*string_allocated);
    printf("The respective brk addr is : %p\r\n",string_allocated);


    //3ème allocation : LONG
    long * long_number_allocated = malloc_3is(BYTES_FOR_CURRENT_MONTH);
    *long_number_allocated = CURRENT_MONTH;
    if(long_number_allocated == (void *) -1){
        perror("malloc_3is : ");
        exit(EXIT_FAILURE);
    }
    printf("Current month is %ld\r\n",*long_number_allocated);
    printf("The respective brk addr is : %p\r\n",long_number_allocated);
    //

    exit(EXIT_SUCCESS);


}

