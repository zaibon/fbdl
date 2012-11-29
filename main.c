#include "fb.h"

int main(int argc, char const *argv[])
{
	list_t *list;

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

	printf("Album de %s\n",list->album.name);
	for (int i = 0; list != NULL; i++)
	{
		printf("[%2d] %s (%d photos)\n",i,list->album.name,list->album.count );
		list = list->next;
	}

	return 0;
}