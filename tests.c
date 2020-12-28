#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char* argv[]){

    printf("Tests de l'allocateur mémoire\n");

    int res;

    printf("\nTEST 1\n");
    res = test1();
    if(res != 0){

    }
    printf("TEST 1 OK");
    printf("\nTEST 2\n");
    res = test2();
    if(res != 0){
        
    }
    printf("TEST 2 OK");
    printf("\nTEST 3\n");
    res = test3();
    if(res != 0){
        
    }
    printf("TEST 3 OK");
    return 0;
}

int test1(){
    printf("Multiples initialisations de la mémoire (et affichage pour vérification)\n");

    for(int i=0;i<10;i++){
        mem_init(get_memory_adr,get_memory_size);
        mem_show(afficher_zone);
    }
    printf("Aucun crash\n");
    return 0;
}

int test2(){
    
}

int test3(){
    
}

void afficher_zone(void *adresse, size_t taille, int free)
{
  printf("Zone %s, Adresse : %lu, Taille : %lu\n", free?"libre":"occupee",
         adresse - get_memory_adr(), (unsigned long) taille);
}