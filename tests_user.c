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
	    assert(ptr != NULL);

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
        printf("Allocation n.%d\n",i);
        assert(ptr != NULL);

        printf("Memoire allouee en %d\n", (int) (ptr-get_memory_adr()));
    }
    printf("allons-y\n");
    printf("Allocations OK\n");
    for(int i=0;i<340;i++){
        mem_free(get_memory_adr() + 32 + i*24);
    }
    printf("La libération ne renvoie rien : Affichage pour vérification manuelle\n");
    mem_show(afficher_zone);
    return 0;
    
}

int test4(){
    printf("Vérification alignement 8 par 8\n");
    // Lorsque l'on demande l'allocaion d'un bloc de taille qui n'est pas modulo 8,
    // on offre de l'espace utilisateur supplémentaire pour conserver l'alignement.
    mem_init(get_memory_adr(),get_memory_size());
    void* ptr = mem_alloc(50);
	assert(ptr != NULL);

    printf("Memoire allouee en %d\n", (int) (ptr-get_memory_adr()));
        
    printf("On a demandé 50 d'espace qui n'est pas un multiple de 8\nOn devrait avoir un bloc de 56\n");
    mem_show(afficher_zone);
    return 0;
}

int test5(){
    printf("Vérification adaptation autocomplétion\n");
    // Si on alloue un bloc de taille 8 dans un bloc libre de taille 16 par exemple, on offre les 8 restant car il n'y a pas la place
    // de créer un nouveau bloc dans les 8 restants.
    mem_init(get_memory_adr(),get_memory_size());
    void* ptr = mem_alloc(16);
	assert(ptr != NULL);
    ptr = mem_alloc(16);
	assert(ptr != NULL);
    printf("On doit avoir deux blocs de 16 alloués et un grand bloc libre\n");
    mem_show(afficher_zone);
    mem_free(get_memory_adr() + 32);
    printf("On doit avoir deux blocs de 16 un libre, un occupé et un grand bloc libre\n");
    mem_show(afficher_zone);
    ptr = mem_alloc(8);
	assert(ptr != NULL);
    printf("On doit avoir de nouveau deux blocs de 16 alloués et un grand bloc libre\n");
    mem_show(afficher_zone);
    return 0;
}

int main(int argc, char* argv[]){

    printf("Tests de l'allocateur mémoire\n");
    printf("\nTEST 1\n");
    test1();
    printf("TEST 1 OK\n");
    printf("\nTEST 2\n");
    test2();
    printf("TEST 2 OK\n");
    printf("\nTEST 3\n");
    test3();
    printf("TEST 3 OK\n");
    printf("\nTEST 4\n");
    test4();
    printf("TEST 4 OK\n");
    printf("\nTEST 5\n");
    test5();
    printf("TEST 5 OK\n");

    return 0;
}