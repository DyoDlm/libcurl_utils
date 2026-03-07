#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "util.h"

char    *get_next_line(int fd);
size_t	WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

static char	*build_argument(char *prompt, char *model)
{
	const char	*close_format = "\", \"stream\":false }  ";
	const char	*json_format  = "{ \"model\":\"";
	const char	*json_format2 = "\",\"prompt\":\"";
	size_t		argument_len = strlen(prompt) 
					+ strlen(json_format) 
					+ strlen(json_format2)
					+ strlen(model)
					+ strlen(close_format) + 1;
	char		*argument = malloc(argument_len);
	
	if (!argument)
		return NULL;
	argument[argument_len] = '\0';
	strlcat(argument, json_format, strlen(argument) + strlen(json_format) + 1);
	strlcat(argument, model, strlen(argument) + strlen(model) + 1);
	strlcat(argument, json_format2, strlen(argument) + strlen(json_format2) + 1);
	strlcat(argument, prompt, strlen(argument) + strlen(prompt) + 1);
	strlcat(argument, close_format, strlen(argument) + strlen(close_format) + 1);
	printf("argument is : %s\n", argument);
	//	argument = url + json_format + prompt + close_format;	
	return argument;
}

static char	*exec(const char *url, char *argument)
{
	CURL *curl_handle;
	CURLcode res;
	char	*response = NULL;
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
			// fprintf(stderr, "error: %s\n", curl_easy_strerror(res));
		} else {
			response = strdup(chunk.memory);
			// printf("Size: %lu\n", (unsigned long)chunk.size);
			// printf("Data: %s\n", chunk.memory);
		}
		curl_easy_cleanup(curl_handle);
		free(chunk.memory);
	}
	return response;
	(void)chunk;
}

static void	clean_up_json(char **s)
{
	const char	*start = ",\"response\":\"";
	const char	*end   = "\",\"done\":";
	char		*cleaned_up = NULL;
	size_t		from = 0;
	size_t		to = 0;
	size_t		iterator = 0;

	if (*s)
		printf("Cleaning : %s\n", *s);
	printf("Start len : %ld\nEnd len : %ld\nTotal len : %ld\n", 
			strlen(start), strlen(end), strlen(*s));
	
	for (size_t i = 0; (*s)[i] && (!from || !to); i++)
	{
		iterator = 0;
		printf("A");
		while (!from
				&& start[iterator] && (*s)[i + iterator]
				&& start[iterator] == (*s)[i + iterator])
			iterator++;
		printf("B");
		while (from
				&& end[iterator] && (*s)[i + iterator]
				&& end[iterator] == (*s)[i + iterator])
			iterator++;
		printf("C");
		switch (iterator) {
			case 13:
				from = i;
				break ;
			case 9:
				to = i;
				break ;
			default:
				break ;
		}
		printf("D");
	}
	printf("extracting from : %ld to %ld\n", from, to);
	cleaned_up = malloc(to - from + 1);
	if (!cleaned_up)
		return ;
	iterator = 0;
	while (from < to)
		cleaned_up[iterator++] = (*s)[from++];
	cleaned_up[iterator] = 0;
	free(*s);
	*s = cleaned_up;
	printf("Cleaned up res 1 : %s\n", cleaned_up);
}

static void	display(char *s)
{
	printf("Cleaned up response : %s\n", s);
	return ; (void)s;
}

int	main(int ac, char **av)	//	./ask_littletown [prompt] [model]
{
	char		*curl_argument = NULL;
	char		*prompt = NULL;
	char		*model = NULL;
	char		*response = NULL;
	const char	*url = "http://192.168.1.200:11434/api/generate";

	if (ac <= 2) {
		return 0;
		printf("No argument given\nEnter you prompt :\n");
		prompt = get_next_line(0);
		if (prompt)
			prompt[strlen(prompt) - 1] = ' ';
		free(prompt);
		return 0;
	} else {
		prompt = strdup(av[1]);
		model = strdup(av[2]);
	}
	curl_argument = build_argument(prompt, model);
	response = exec(url, curl_argument);
	clean_up_json(&response);
	display(response);
	free(response);
	//printf("Prompt : %s\n", prompt);
	free(prompt);
	free(model);
	if (curl_argument)
		free(curl_argument);
	return 1;
}

