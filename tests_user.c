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
    printf("\nMultiples initialisations de la mémoire (et affichage pour vérification)\n");

    for(int i=0;i<10;i++){
        mem_init(get_memory_adr(),get_memory_size());
        mem_show(afficher_zone);
    }
    printf("\nAucun crash\n\n");

    return 0;
}

int test2(){

        printf("\nAllocation et libération MAX\n\n");
        mem_init(get_memory_adr(),get_memory_size());
        void* ptr = mem_alloc(get_memory_size() - 48);
	    assert(ptr != NULL);
        
        mem_show(afficher_zone);

        mem_free(get_memory_adr() + 32);
        printf("\nLa libération ne renvoie rien : Affichage pour vérification manuelle\n");
        mem_show(afficher_zone);

        return 0;
}

int test3(){
    printf("\nAllocation du nombre max de blocs de taille 8 puis libération\n\n");
    mem_init(get_memory_adr(),get_memory_size());

    for(int i=0;i<340;i++){
        void* ptr = mem_alloc(8);
        printf("Allocation n.%d\n",i+1);
        assert(ptr != NULL);

        printf("Memoire allouee en %d\n", (int) (ptr-get_memory_adr()));
    }
    printf("\nAllocations OK\n\n");
    for(int i=0;i<340;i++){
        mem_free(get_memory_adr() + 32 + i*24);
    }
    printf("La libération ne renvoie rien : Affichage pour vérification manuelle\n");
    mem_show(afficher_zone);

    return 0;
    
}

int test4(){
    printf("\nVérification alignement 8 par 8\n");
    // Lorsque l'on demande l'allocaion d'un bloc de taille qui n'est pas modulo 8,
    // on offre de l'espace utilisateur supplémentaire pour conserver l'alignement.
    mem_init(get_memory_adr(),get_memory_size());
    void* ptr = mem_alloc(50);
	assert(ptr != NULL);

    printf("Memoire allouee en %d\n", (int) (ptr-get_memory_adr()));
        
    printf("On a demandé 50 d'espace qui n'est pas un multiple de 8\nOn devrait avoir un bloc de 56\n\n");
    mem_show(afficher_zone);

    return 0;
}

int test5(){
    printf("\nVérification adaptation autocomplétion\n\n");
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
    printf("\nOn doit avoir deux blocs de 16 un libre, un occupé et un grand bloc libre\n");
    mem_show(afficher_zone);
    ptr = mem_alloc(8);
	assert(ptr != NULL);
    printf("\nOn doit avoir de nouveau deux blocs de 16 alloués et un grand bloc libre\n");
    mem_show(afficher_zone);

    return 0;
}

int test6(){
    printf("\nVérification fusion des blocs libres lors de la libération\n\n");

    mem_init(get_memory_adr(),get_memory_size());

    void* ptr = mem_alloc(16);
	assert(ptr != NULL);
    mem_show(afficher_zone);
    printf("Lorsqu'on libère le bloc occupé il doit fusionner avec le bloc suivant\n\n");
    mem_free(get_memory_adr() + 32);
    printf("La libération ne renvoie rien : Affichage pour vérification manuelle\n");
    mem_show(afficher_zone);

    printf("\nOn alloue maintenant 3 blocs et on libère le premier\n");
    ptr = mem_alloc(16);
	assert(ptr != NULL);
    ptr = mem_alloc(16);
	assert(ptr != NULL);
    ptr = mem_alloc(16);
	assert(ptr != NULL);
    mem_free(get_memory_adr() + 32);
    mem_show(afficher_zone);
    printf("\nLorsque l'on va libérer le premier bloc occupé (deuxième bloc en général),\nil va fusionner avec le bloc libre qui le précède\n");
    mem_free(get_memory_adr() + 96);
    printf("La libération ne renvoie rien : Affichage pour vérification manuelle\n");
    mem_show(afficher_zone);

    printf("\nIl reste un bloc occupé encadré par deux blocs libres.\nA sa libération, le tout devrait fusionner et on devrait avoir un unique bloc libre de taille MAX\n");
    mem_free(get_memory_adr() + 64);
    printf("La libération ne renvoie rien : Affichage pour vérification manuelle\n");
    mem_show(afficher_zone);

    return 0;
}

int test7(){
    printf("\nTest allocations impossibles\n\n");
    mem_init(get_memory_adr(),get_memory_size());

    printf("Allocation de taille > MAX (MAX = 8144)\n");
    void * ptr = mem_alloc(9000);
    assert(ptr == NULL);
    printf("OK\n\n");

    printf("Allocation de taille < 0\n");
    // Etant donné que le type size_t est analogue d'un long UNSIGNED, il n'y a pas de nombre négatif.
    // Le nombre donné est donc converti en un nombre positif très grand, bien souvent trop.
    ptr = mem_alloc(9000);
    assert(ptr == NULL);
    printf("OK\n\n");

    printf("Allocation valide mais toute la mémoire est occupée\n");
    ptr = mem_alloc(get_memory_size() - 48);
    assert(ptr != NULL);
    mem_show(afficher_zone);
    printf("Toute la mémoire est occupée\n");
    ptr = mem_alloc(48);
    assert(ptr == NULL);
    printf("OK\n\n");

    return 0;
}

int test8(){
    printf("\nTest libérations impossibles\n\n");
    mem_init(get_memory_adr(),get_memory_size());

    printf("On commence par allouer 3 blocs\n");
    void * ptr = mem_alloc(48);
    assert(ptr != NULL);
    ptr = mem_alloc(48);
    assert(ptr != NULL);
    ptr = mem_alloc(48);
    assert(ptr != NULL);
    mem_show(afficher_zone);

    printf("\nDébut des tests :\n");

    printf("Libération d'une zone post-mémoire :\n");
    mem_free(get_memory_adr() + get_memory_size());
    printf("\nLibération d'une zone pré-mémoire :\n");
    mem_free(get_memory_adr() - 48);
    printf("\nLibération à une adresse dans la mémoire mais ne correspondant pas au début d'un bloc :\n");
    mem_free(get_memory_adr() + 25);

    return 0;
}

int main(int argc, char* argv[]){

    printf("Tests de l'allocateur mémoire\n");
    printf("\nTEST 1\n");
    test1();
    printf("TEST 1 OK\n");
    printf("\nTEST 2\n");
    test2();
    printf("\nTEST 2 OK\n");
    printf("\nTEST 3\n");
    test3();
    printf("\nTEST 3 OK\n");
    printf("\nTEST 4\n");
    test4();
    printf("\nTEST 4 OK\n");
    printf("\nTEST 5\n");
    test5();
    printf("\nTEST 5 OK\n");
    printf("\nTEST 6\n");
    test6();
    printf("\nTEST 6 OK\n");
    printf("\nTEST 7\n");
    test7();
    printf("TEST 7 OK\n");
    printf("\nTEST 8\n");
    test8();
    printf("\nTEST 8 OK\n");

    return 0;
}