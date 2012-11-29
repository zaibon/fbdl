#ifndef FB_H
#define FB_H 

#include <stdio.h>

#define FB_TOKEN ""
#define FB_QUERY_URL "https://api.facebook.com/method/fql.query?query=%s&access_token=%s&format=json"
#define URL_SIZE 1024

#define QUERY_ALBUM "SELECT aid,name,photo_count,owner FROM album WHERE owner IN (select uid from user WHERE username = '%s')"
#define QUERY_PHOTO "SELECT src_big,pid FROM photo WHERE aid = '%s' LIMIT %d"

#define BUFFER_SIZE  (256 * 1024)  /* 256 KB */

typedef struct
{
	char aid[256];
	unsigned int uid;
	char name[256];
	unsigned int count;
}album_t;

typedef struct
{
	char url[256];
	char pid[256];
}photo_t;

struct list_t
{
	void *element;
	struct list_t* next;
};
typedef struct list_t list_t;

typedef struct
{
    char *data;
    int pos;
} write_result;


size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream);
char* request(const char* url);
list_t* getAlbums(const char* username);
list_t* getPhotos(char* aid, unsigned int count);
int downloadPhotos(list_t *listPhoto, const char* dest);
void freeList(list_t *list);
#endif	