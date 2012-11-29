#include "fb.h"

int main(int argc, char const *argv[])
{
	list_t *list;
	album_t *album;

	if(argc != 2){
		printf("usage : fbdl username|pagename\n");
		return 1;
	}

	printf("recupération album de %s\n",argv[1] );
	list = getAlbums(argv[1]);
	if(list == NULL)
	{
		fprintf(stderr, "%s\n", "Erreur recupération album");
		return 1;
	}

	album = list->element;
	printf("Album de %s\n",album->name);
	for (int i = 0; list != NULL; i++)
	{
		printf("[%2d] %s (%d photos)\n",i,album->name,album->count );
		list = list->next;
		if(list != NULL)
			album = list->element;
	}

	freeList(list);
	return 0;
}