/* copyright (c) 2007 magnus auvinen, see licence.txt for more info */

typedef struct CHUNK_t
{
	char *memory;
	char *current;
	char *end;
	struct CHUNK_t *next;
} CHUNK;

typedef struct 
{
	CHUNK *current;
} HEAP;

HEAP *memheap_create();
void memheap_destroy(HEAP *heap);
void *memheap_allocate(HEAP *heap, unsigned int size);
