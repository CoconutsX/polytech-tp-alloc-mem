//Test branchage Corentin -> OK
// Depot distant : TP-Allocateur-Memoire
/* On inclut l'interface publique */
#include "mem.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/* Définition de l'alignement recherché
 * Avec gcc, on peut utiliser __BIGGEST_ALIGNMENT__
 * sinon, on utilise 16 qui conviendra aux plateformes qu'on cible
 */
#ifdef __BIGGEST_ALIGNMENT__
#define ALIGNMENT __BIGGEST_ALIGNMENT__
#else
#define ALIGNMENT 16
#endif

/* structure placée au début de la zone de l'allocateur
   Elle contient toutes les variables globales nécessaires au
   fonctionnement de l'allocateur
   Elle peut bien évidemment être complétée
*/
struct allocator_header {
        size_t memory_size;
	mem_fit_function_t *fit;
	struct fb* first_fb;
	struct bb* first_bb;
};

/* La seule variable globale autorisée
 * On trouve à cette adresse le début de la zone à gérer
 * (et une structure 'struct allocator_header)
 */
static void* memory_addr;

static inline void *get_system_memory_addr() {
	return memory_addr;
}

static inline struct allocator_header *get_header() {
	struct allocator_header *h;
	h = get_system_memory_addr();
	return h;
}

static inline size_t get_system_memory_size() {
	return get_header()->memory_size;
}

// Liste chainée des blocs libres
struct fb {
	size_t size;
	// void* memory_addr;
	struct fb* next;
};

// Liste chainée des blocs occupés
struct bb {
	size_t size;
	// void* memory_addr;
	struct bb* next;

	/* size_user est la taille des données stockées. Elle est
	 * différente de la taille utilisable pour les données
	 * qui est (size - sizeof(struct fb))
	 */
	// size_t size_user;
};



void mem_init(void* mem, size_t taille)
{
    memory_addr = mem;
    *(size_t*)memory_addr = taille;
	/* On vérifie qu'on a bien enregistré les infos et qu'on
	 * sera capable de les récupérer par la suite
	 */

	assert(mem == get_system_memory_addr());
	assert(taille == get_system_memory_size());

	/* Paramètrage de la tête de mémoire, contenant les infos
	 * générales sur la zone mémoire
	 */
	struct allocator_header* h;
	h = (struct allocator_header*)mem;
	h->memory_size = taille;
	h->first_fb = (struct fb*)(h+1);
	h->first_bb = NULL; // Aucune zone occupée à l'initialisation
	mem_fit(&mem_fit_first);

	/* Paramétrage de l'unique bloc libre qu'on a pour l'instant
	 */
	struct fb* zoneLibre = h->first_fb;
	zoneLibre->size = taille - sizeof(struct allocator_header);
	zoneLibre->next = NULL;
}

void mem_show(void (*print)(void *zone, size_t size, int free)) {
    struct allocator_header* h = get_header();
    struct fb* free_block_ptr = h->first_fb;
	struct bb* busy_block_ptr = h->first_bb;
    while (free_block_ptr != NULL || busy_block_ptr != NULL) {
        if (busy_block_ptr == NULL || free_block_ptr < busy_block_ptr) {
			print(free_block_ptr, free_block_ptr->size, 1);
			free_block_ptr = free_block_ptr->next;
		} 
		else
		{
			print(busy_block_ptr, busy_block_ptr->size, 0);
			busy_block_ptr = busy_block_ptr->next;
		}
    }
}

void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

void *mem_alloc(size_t taille) {
	/* ... */
	__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */
	struct fb *fb=get_header()->fit(get_header()->first_fb, taille - sizeof(struct fb));
	/* ... */
	return NULL;
}


void mem_free(void* mem) {
}


struct fb* mem_fit_first(struct fb *list, size_t size) {
	struct fb* zone_browser = get_header()->first_fb;
	while (zone_browser != NULL) {
		if (zone_browser->size - sizeof(struct fb) >= size) {
			return zone_browser;
		}
		zone_browser = zone_browser->next;
	}
	return zone_browser; // NULL
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	/* zone est une adresse qui a été retournée par mem_alloc() */

	/* la valeur retournée doit être la taille maximale que
	 * l'utilisateur peut utiliser dans cette zone */
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb *list, size_t size) {
	return NULL;
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	return NULL;
}