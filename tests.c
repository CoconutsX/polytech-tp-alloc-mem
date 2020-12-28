#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void afficher_zone(void *adresse, size_t taille, int free)
{
  printf("Zone %s, Adresse : %lu, Taille : %lu\n", free?"libre":"occupee",
         adresse - get_memory_adr(), (unsigned long) taille);
}

int test1(){
    printf("Multiples initialisations de la mémoire (et affichage pour vérification)\n");

    for(int i=0;i<10;i++){
        mem_init(get_memory_adr(),get_memory_size());
        mem_show(afficher_zone);
    }
    printf("Aucun crash\n");
    return 0;
}

int test2(){

        printf("Allocation et libération MAX\n");
        mem_init(get_memory_adr(),get_memory_size());
        void* ptr = mem_alloc(get_memory_size() - 48);
	    if (ptr == NULL)
            return 1;
        else
            printf("Memoire allouee en %d\n", (int) (ptr-get_memory_adr()));
        
        mem_show(afficher_zone);

        mem_free(get_memory_adr() + 32);
        printf("La libération ne renvoie rien : Affichage pour vérification manuelle\n");
        mem_show(afficher_zone);
        return 0;
}

int test3(){
    printf("Allocation du nombre max de blocs de taille 8 puis libération\n");
    mem_init(get_memory_adr(),get_memory_size());

    for(int i=0;i<340;i++){
        void* ptr = mem_alloc(8);
        if (ptr == NULL){
            if(i == 0){
                i--;
            }
            return i;
        }
        else
            printf("Memoire allouee en %d\n", (int) (ptr-get_memory_adr()));
    }
    printf("Allocations OK\n");
    for(int i=0;i<340;i++){
        mem_free(get_memory_adr() + 32 + i*24);
    }
    printf("La libération ne renvoie rien : Affichage pour vérification manuelle\n");
    mem_show(afficher_zone);
    return 0;
    
}

int main(int argc, char* argv[]){

    printf("Tests de l'allocateur mémoire\n");

    int res;

    printf("\nTEST 1\n");
    res = test1();
    printf("TEST 1 OK");
    printf("\nTEST 2\n");
    res = test2();
    if(res == 1){
        printf("Allocation MAX impossible\n");
        return -1;        
    }
    printf("TEST 2 OK");
    printf("\nTEST 3\n");
    res = test3();
    if(res != 0){
        if(res == -1){
            res++;
        }
        printf("Allocation n.%d/340 échouée\n",res);
        return -1;
    }
    printf("TEST 3 OK");
    return 0;
}