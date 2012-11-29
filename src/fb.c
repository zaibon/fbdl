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
    	

    	tmp = (list_t*) malloc(sizeof(list_t));

        strncpy(tmp->album.aid, json_string_value(aid), 256);
    	tmp->album.uid = json_integer_value(uid);    	
        strncpy(tmp->album.name, json_string_value(name), 256);
    	tmp->album.count = json_integer_value(count);
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