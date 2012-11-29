#include "fb.h"

#include <curl/curl.h>
#include <jansson.h>
#include <string.h>
#include <stdlib.h>

size_t write_response(void *ptr, size_t size, size_t nmemb, void *stream)
{
    write_result *result = (write_result *)stream;

    if(result->pos + size * nmemb >= BUFFER_SIZE - 1)
    {
        fprintf(stderr, "error: too small buffer\n");
        return 0;
    }

    memcpy(result->data + result->pos, ptr, size * nmemb);
    result->pos += size * nmemb;

    return size * nmemb;
}

char *request(const char *url)
{
    CURL *curl;
    CURLcode status;
    char *data;
    long code;

    curl = curl_easy_init();
    data = malloc(BUFFER_SIZE);
    if(!curl || !data)
        return NULL;

    write_result write_result = {
        .data = data,
        .pos = 0
    };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_result);

    status = curl_easy_perform(curl);
    if(status != 0)
    {
        fprintf(stderr, "error: unable to request data from %s:\n", url);
        fprintf(stderr, "%s\n", curl_easy_strerror(status));
        return NULL;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if(code != 200)
    {
        fprintf(stderr, "error: server responded with code %ld\n", code);
        return NULL;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    /* zero-terminate the result */
    data[write_result.pos] = '\0';

    return data;
}

list_t* getAlbums(const char* username)
{
	char url[URL_SIZE];
    char query[URL_SIZE];
	char* response = NULL;

	json_error_t error;
	json_t *root;

	json_t *album;
	json_t *aid;
	json_t *uid;
	json_t *name;
	json_t *count;

    album_t *content;

	list_t *list = NULL;
	list_t* tmp = NULL;
	list_t* prev = NULL;

    CURL *curl;

    if( (curl = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "%s\n", "erreur curl_easy_init");
        return NULL;
    }

    snprintf(query, URL_SIZE, QUERY_ALBUM,username);
    strcpy(query,curl_easy_escape(curl, query, 0));
	snprintf(url,URL_SIZE, FB_QUERY_URL,query,FB_TOKEN);
    
    curl_easy_cleanup(curl);

	response = request(url);
	if(!response)
    {
        fprintf(stderr, "%s\n", "Erreur request");
		return NULL;
    }

	root = json_loads(response,0,&error);
	if(!root)
	free(response);

	if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }


    for (int i = 0; i < json_array_size(root); i++)	
    {
    	album = json_array_get(root, i);

    	aid = json_object_get(album, "aid");
    	uid = json_object_get(album, "owner");
    	name = json_object_get(album, "name");
    	count = json_object_get(album, "photo_count");
    	

        content = (album_t*) malloc(sizeof(album_t));
    	tmp = (list_t*) malloc(sizeof(list_t));
        tmp->element = content;

        strncpy(content->aid, json_string_value(aid), 256);
    	content->uid = json_integer_value(uid);    	
        strncpy(content->name, json_string_value(name), 256);
    	content->count = json_integer_value(count);
    	tmp->next = NULL;

    	if(list == NULL) /* premier passage*/
		{	
    		list = tmp;
    		prev = tmp;
    	}
    	else
    	{
    		prev->next = tmp;
    		prev = tmp;
    	}
    }

    json_decref(root);
    return list;
}

list_t* getPhotos(char* aid, unsigned int count)
{
    char url[URL_SIZE];
    char query[URL_SIZE];
    char* response = NULL;

    json_error_t error;
    json_t *root;
    json_t *photo;

    json_t *src;
    json_t *pid;

    photo_t *content;

    list_t *list = NULL;
    list_t* tmp = NULL;
    list_t* prev = NULL;

    CURL *curl;
    if( (curl = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "%s\n", "erreur curl_easy_init");
        return NULL;
    }

    snprintf(query, URL_SIZE, QUERY_PHOTO,aid,count);
    strcpy(query,curl_easy_escape(curl, query, 0));
    snprintf(url,URL_SIZE, FB_QUERY_URL,query,FB_TOKEN);
    
    curl_easy_cleanup(curl);

    response = request(url);
    if(!response)
    {
        fprintf(stderr, "%s\n", "Erreur request");
        return NULL;
    }

    root = json_loads(response,0,&error);
    if(!root)
    free(response);

    if(!root)
    {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return NULL;
    }

    for (int i = 0; i < json_array_size(root); i++)
    {
        photo = json_array_get(root, i);

        src = json_object_get(photo, "src_big");
        pid = json_object_get(photo, "pid");
        

        content = (photo_t*) malloc(sizeof(photo_t));
        tmp = (list_t*) malloc(sizeof(list_t));
        tmp->element = content;

        strncpy(content->url, json_string_value(src), 256);
        strncpy(content->pid, json_string_value(pid), 256);
        tmp->next = NULL;

        if(list == NULL) /* premier passage*/
        {   
            list = tmp;
            prev = tmp;
        }
        else
        {
            prev->next = tmp;
            prev = tmp;
        }
    }

    json_decref(root);
    return list;
}

int downloadPhotos(list_t *listPhoto, const char* dest)
{
    CURL *curl;
    CURLcode status;

    list_t *run = listPhoto;
    photo_t *photo = NULL;

    char path[256];
    FILE *file;

    int count = 0;

    if( (curl = curl_easy_init()) == NULL)
    {
        fprintf(stderr, "%s\n", "erreur curl_easy_init");
        return -1;
    }

    while(run != NULL)
    {   
        photo = run->element;

        sprintf(path, "%s/%s.jpg",dest,photo->pid);
        if( (file = fopen(path, "w")) == NULL)
        {
            perror("error fopen");
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, photo->url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA,file);

        status = curl_easy_perform(curl);

        if(status != 0)
        {
            fprintf(stderr, "error: unable to request data from %s:\n", photo->url);
            fprintf(stderr, "%s\n", curl_easy_strerror(status));
            return -1;
        }

        if(fclose(file) == -1)
        {
            perror("error fclose");
            return -1;
        }        
        count++;

        run = run->next;
    }

    curl_easy_cleanup(curl);
    return count;
}

void freeList(list_t *list)
{
    list_t *tmp = list;
    list_t *prev = NULL;

    while(tmp != NULL)
    {
        prev = tmp;
        tmp = tmp->next;
        free(prev->element);
        free(prev);
    }
}