#include "fb.h"
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char const *argv[])
{
	list_t *listAlbum = NULL;
	list_t *listPhoto = NULL;
	list_t *run = NULL;
	album_t *album = NULL;
	char buffer[4];
	int running = 1;

	if(argc != 2){
		printf("usage : fbdl username|pagename\n");
		return 1;
	}

	printf("recupération album de %s\n",argv[1] );
	listAlbum = getAlbums(argv[1]);
	if(listAlbum == NULL)
	{
		fprintf(stderr, "%s\n", "Erreur recupération album");
		return 1;
	}

	while(running)
	{

		album = listAlbum->element;
		printf("Album de %s\n",album->name);

		run = listAlbum;
		for (int i = 0; run != NULL; i++)
		{
			printf("[%2d] %s (%d photos) - %s\n",i,album->name,album->count,album->aid);
			run = run->next;
			if(run != NULL)
				album = run->element;
		}

		printf("[q] quitter\n");
		printf("\n Quel album voulez vous télécharger ? : ");
		fgets(buffer, 4, stdin);

		if(buffer[0] == 'q')
			break;

		run = listAlbum;
		for (int i = 0; i < atoi(buffer); ++i)
		{
			run = run->next;
		}

		album = run->element;
		printf("téléchargement de l'album %s (%d photos)\n",album->name,album->count );
		listPhoto = getPhotos(album->aid, album->count);

		if( mkdir(album->name, 0775) == -1)
		{
			perror("error mkdir");
			return 1;
		}

		if( downloadPhotos(listPhoto,album->name) == -1)
		{
			fprintf(stderr, "error download photo\n");
			return 1;
		}
	}


	freeList(listAlbum);
	if(listPhoto != NULL)
		free(listPhoto);
	return 0;
}