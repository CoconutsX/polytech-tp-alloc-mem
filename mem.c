//Test branchage Corentin -> OK
// Depot distant : TP-Allocateur-Memoire
/* On inclut l'interface publique */
#include "mem.h"

#include <stdio.h>
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

/*
 * Contient notamment deux champs de tête de listes chainées, une de blocs libres et l'autre de
 * blocs occupés. Cela facilite la gestion de la mémoire et son parcours.
 */
struct allocator_header {
    size_t memory_size;
	mem_fit_function_t *fit;
	struct fb* first_fb;
	struct fb* first_bb;
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

struct fb
{
	// Taille UTILISATEUR
	size_t size;
	// Bloc suivant (libre ou occupé selon la nature de ce bloc)
	struct fb* next;
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
	zoneLibre->size = taille - sizeof(struct allocator_header) - sizeof(struct fb);
	zoneLibre->next = NULL;
}

void mem_show(void (*print)(void *zone, size_t size, int free)) {
    struct allocator_header* h = get_header();

	// On récupère un pointeur sur les deux files
    struct fb* free_fb_ptr = h->first_fb;
	struct fb* busy_fb_ptr = h->first_bb;

	// Tant qu'il reste des blocs à traiter
    while (free_fb_ptr != NULL || busy_fb_ptr != NULL) {

		// Si il n'y a plus de bloc occupé ou que le bloc libre actuel est avant
        if (busy_fb_ptr == NULL || (free_fb_ptr != NULL && free_fb_ptr < busy_fb_ptr)) {

			// On affiche le bloc libre actuel et on passe au bloc libre suivant
			print(free_fb_ptr, free_fb_ptr->size, 1);
			free_fb_ptr = free_fb_ptr->next;
		} 
		else
		{
			// Sinon on affiche le bloc occupé actuel et on passe au suivant
			print(busy_fb_ptr, busy_fb_ptr->size, 0);
			busy_fb_ptr = busy_fb_ptr->next;
		}
    }
}

// Récupère la fonction "fit" dans l'entête de la mémoire
void mem_fit(mem_fit_function_t *f) {
	get_header()->fit = f;
}

void *mem_alloc(size_t taille) {

	// On augmente au cas où la taille demandée pour respecter l'alignement de 8 en 8.
	if(taille % 8 != 0){
		taille += 8 - (taille % 8);
	}

	//__attribute__((unused)) /* juste pour que gcc compile ce squelette avec -Werror */

	// fit se charge de récupérer le premier bloc qui correspond à la taille demandée
	struct fb *fb=get_header()->fit(get_header()->first_fb, taille);

	if (fb == NULL) {
		fprintf(stderr, "Allocation impossible : aucun bloc de taille demandée trouvé.\n");
		return NULL;
	}

	// On récupère le bloc libre précédent le bloc qu'on va allouer (ou le premier si ce dernier est le premier)
	struct fb *fb_parser = get_header()->first_fb;
	while(fb_parser->next<fb && fb_parser->next != NULL)
	{
		fb_parser = fb_parser->next;
	}

	// idem pour le bloc occupé
	struct fb *bb_parser = get_header()->first_bb;
	while(bb_parser != NULL && bb_parser->next<fb && bb_parser->next != NULL)
	{
		bb_parser = bb_parser->next;
	}

	// On recupere le bloc supposé libre après fb une fois son allocation (ce qui n'est pas utilisé dans fb)
	long next_free_block_addr;
	next_free_block_addr = (long) fb + taille + sizeof(struct fb);
	struct fb* next_free_block = (struct fb*) next_free_block_addr;
	
	// Si il se trouve que cet espace est un bloc occupé, on prend simplement fb->next
	if(bb_parser != NULL && (next_free_block == bb_parser->next || next_free_block == bb_parser)){
		next_free_block = fb->next;
	}
	else
	{
		if(next_free_block == (get_system_memory_addr() + get_system_memory_size())){
			// Si le bloc suivant est après la mémoire, next_free_block = NULL
			next_free_block = NULL;
		}
		else{
			// si le bloc suivant est la partie non allouée suivant fb, on change sa taille
			next_free_block->size = fb->size - taille - sizeof(struct fb);
			next_free_block->next = fb->next;
		}
	}
	// On a maintenant le bloc libre suivant fb à coup sur

	// On actualise la liste des blocs libres
	if(fb_parser != fb){
		fb_parser->next = next_free_block;
	}
	else
	{
		get_header()->first_fb = next_free_block;
	}

	// On actualise la liste des blocs occupés
	if(bb_parser != NULL && bb_parser < fb){
		fb->next = bb_parser->next;
		fb->size = taille;
		bb_parser->next = fb;
	}
	// Cas où la première zone occupée se trouvait après la nouvelle zone allouée
	else if (bb_parser != NULL)
	{
		get_header()->first_bb = fb;
		fb->next = bb_parser;
		fb->size = taille;
	}
	else
	{
		get_header()->first_bb = fb;
		fb->next = NULL;
		fb->size = taille;
	}

	// On renvoie l'adresse de la zone utilisateur
	long res = (long) fb;
	res += sizeof(struct fb);
	return (void *) res;
}



void mem_free(void* mem) {

	// On parcourt la liste des blocs libres
	struct fb *fb_parser = get_header()->first_fb;
	while (fb_parser != NULL && fb_parser != mem)
	{
		fb_parser = fb_parser->next;
	}

	// Si on a rencontré le bloc que l'on veut libérer, il n'y a rien à faire
	if (fb_parser == mem)
	{
		printf("La zone est déjà libre, aucune action recquise.\n");
		return;
	}

	// On recherche le bloc que l'on veut libérer dans la liste des occupés
	struct fb *bb_parser = get_header()->first_bb;
	while (bb_parser != NULL && bb_parser != mem)
	{
		bb_parser = bb_parser->next;
	}

	// Si on ne l'a pas trouvé, on ne peut pas le libérer
	if (bb_parser == NULL)
	{
		fprintf(stderr, "Zone introuvable dans la liste des zones occupées, libération impossible.\n");
		return;
	}

	struct fb *zoneToFree = bb_parser;

	// On cherche si les blocs directement avant et après notre bloc à libérer sont libres
	int isZoneBeforeFree = -1;
	int isZoneAfterFree = -1;

	struct fb *bb_before = NULL;
	struct fb *bb_after = NULL;

	bb_before = get_header()->first_bb;

	// Si le bloc à libérer est le premier bloc occupé, il n'y a pas de bloc occupé précédent
	if (bb_before == zoneToFree) {
		bb_before = NULL;
	}
	// Sinon on peut commencer la recherche du bloc occupé précédent
	while (bb_before != NULL && bb_before->next != zoneToFree)
	{
		bb_before = bb_before->next;
	}

	// Récupérer le bloc occupé suivant est assez simple
	bb_after = zoneToFree->next;

	// On observe le premier bloc libre
	struct fb *fb_before = get_header()->first_fb;

	// Si la zone à libérer se trouve avant la première zone libre, il ne peut pas y avoir de zone libre avant elle.
	if (fb_before == NULL || fb_before > bb_parser) 
	{
		fb_before = NULL;
		isZoneBeforeFree = 0;
	}
	else 
	{
		// Sinon on peut commencer la recherche du bloc libre précédent
		while (fb_before->next != NULL && fb_before->next < bb_parser) {
			fb_before = fb_before->next;
		}

		// On compare la position de cette zone avec celle du bloc occupé précédent et on met à jour nos variables en fonction
		if (bb_before == NULL || bb_before < fb_before)
		{
			isZoneBeforeFree = 1;
		} 
		else
		{
			isZoneBeforeFree = 0;
		}
	}

	/*
		RECHERCHE DE fb_after A OPTIMISER :
		si fb_before != NULL, fb_after = fb_before->next
	*/

	// On va commencer la recherche du bloc libre suivant
	struct fb *fb_after = get_header()->first_fb;

	// On retrouve d'abord le bloc libre précédent (ou à défaut le premier si c'est aussi le dernier de la liste)
	while (fb_after != NULL && fb_after < bb_parser)
	{
		fb_after = fb_after->next;
	}

	// Si on a trouvé un bloc nul alors il n'y a pas de bloc libre suivant
	if (fb_after == NULL)
	{
		isZoneAfterFree = 0;
	}
	else
	{
		// Sinon on compare avec le bloc occupé suivant comme précédemment
		if (bb_after == NULL || bb_after > fb_after)
		{
			isZoneAfterFree = 1;
		}
		else
		{
			isZoneAfterFree = 0;
		}
	}

	// LIBERATION MEMOIRE
	// Il s'agit de déplacer le bloc à libérer de la liste des blocs occupés à celle des blocs libres et de raccrocher les liens correctement

	// On lie le bloc précédent au suivant (si il n'y a pas de précédent, le premier devient le suivant (ou NULL si il n'y en a plus))
	if (bb_before == NULL)
	{
		get_header()->first_bb = bb_after;
	}
	else
	{
		bb_before->next = bb_after;
	}

	// Si il n'y a pas de bloc libre précédent le notre on lie notre bloc au premier libre et on définit notre bloc comme le premier libre
	if (fb_before == NULL)
	{
		get_header()->first_fb = zoneToFree;
	}

	// Sinon on relie les blocs libres
	else {
		zoneToFree->next = fb_after;
	}

	zoneToFree->next = fb_after;


	// Eventuelles fusions des blocs libres (on adapte la taille des blocs et on refait les liens correctement)
	if (isZoneAfterFree == 1)
	{
		zoneToFree->size = zoneToFree->size + fb_after->size + sizeof(struct fb);
		zoneToFree->next = fb_after->next;
	}
	if (isZoneBeforeFree == 1)
	{
		fb_before->size = fb_before->size + zoneToFree->size + sizeof(struct fb);
		fb_before->next = zoneToFree->next;
		zoneToFree = fb_before;
	}
}


struct fb* mem_fit_first(struct fb *list, size_t size) {

	// On parcourt les zones libres
	struct fb* zone_browser = list;
	while (zone_browser != NULL) {

		// Si on trouve une zone suffisamment grande on la renvoie
		if (zone_browser->size >= size) {
			return zone_browser;
		}
		zone_browser = zone_browser->next;
	}
	// Si on a rien trouvé, on renvoie NULL
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
	struct fb *bb = zone;
	if (bb == NULL){
		return 0;
	}
	return bb->size;
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb *list, size_t size) {
	
	// On parcourt les zones libres
	struct fb* res = NULL;
	struct fb* zone_browser = list;
	while (zone_browser != NULL) {

		// Si on trouve une zone suffisamment grande
		if (zone_browser->size >= size) {

			// Et que notre résultat actuel est nul ou plus grand, on le prend comme résultat
			if(res == NULL || zone_browser < res){
				res = zone_browser;
			}
		}
		zone_browser = zone_browser->next;
	}
	// Si on a rien trouvé, on renvoie NULL, sinon le bloc le plus petit adapté
	return res; 
}

struct fb* mem_fit_worst(struct fb *list, size_t size) {
	
	// On parcourt les zones libres
	struct fb* res = NULL;
	struct fb* zone_browser = list;
	while (zone_browser != NULL) {

		// Si on trouve une zone suffisamment grande
		if (zone_browser->size >= size) {

			// Et que notre résultat actuel est nul ou plus petit, on le prend comme résultat
			if(res == NULL || zone_browser > res){
				res = zone_browser;
			}
		}
		zone_browser = zone_browser->next;
	}
	// Si on a rien trouvé, on renvoie NULL, sinon le bloc le plus grand adapté
	return res; 
}