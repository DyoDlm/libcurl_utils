#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "util.h"

char    *get_next_line(int fd);
size_t	WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

char	*build_argument(char *prompt)
{
	const char	*close_format = "\", \"stream\":false }  ";
	const char	*json_format  = "{ \"model\":\"mistral:7b\", \"prompt\":\"\"";
	size_t		argument_len = strlen(prompt) 
					+ strlen(json_format) 
					+ strlen(close_format) + 1;
	char		*argument = malloc(argument_len);
	
	if (!argument)
		return NULL;
	argument[argument_len] = 'a';
	strlcat(argument, json_format, strlen(argument) + strlen(json_format));
	strlcat(argument, prompt, strlen(argument) + strlen(prompt));
	strlcat(argument, close_format, strlen(argument) + strlen(close_format));
	printf("argument is : %s\n", argument);
	//	argument = url + json_format + prompt + close_format;	
	return argument;
}

int	exec(const char *url, char *argument)
{
	CURL *curl_handle;
	CURLcode res;

	struct MemoryStruct chunk;
	
	chunk.memory = malloc(1);
	chunk.size = 0;

	curl_handle = curl_easy_init();
	if (curl_handle)
	{
		curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, argument); // "{\"model\":\"mistral:7b\", \"prompt\":\"foo\", \"stream\":false}");
		//curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, "{\"model\":\"mistral:7b\", \"prompt\":\"foo\", \"stream\":false}");

		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		res = curl_easy_perform(curl_handle);

		if (res != CURLE_OK) {
			fprintf(stderr, "error: %s\n", curl_easy_strerror(res));
		} else {
			printf("Size: %lu\n", (unsigned long)chunk.size);
			printf("Data: %s\n", chunk.memory);
		}
		curl_easy_cleanup(curl_handle);
		free(chunk.memory);
	}
	return 0;
	(void)chunk;
}


int	main(int ac, char **av)
{
	char		*curl_argument = NULL;
	char		*prompt = NULL;
	const char	*url = "http://192.168.1.200:11434/api/generate";

	if (ac != 2) {
		printf("No argument given\nEnter you prompt :\n");
		prompt = get_next_line(0);
		if (prompt)
			prompt[strlen(prompt) - 1] = ' ';
	} else {
		prompt = strdup(av[1]);
	}
	printf("Prompt : %s\n", prompt);
	curl_argument = build_argument(prompt);

	exec(url, curl_argument);
	free(prompt);
	if (curl_argument)
		free(curl_argument);
	return 0;
}

