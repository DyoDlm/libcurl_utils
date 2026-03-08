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
	const char	*json_format = "{ \"model\":\"";
	const char	*json_format2 = "\",\"prompt\":\"";
	size_t		argument_len = strlen(prompt)
					+ strlen(json_format)
					+ strlen(json_format2)
					+ strlen(model) 
					+ strlen(close_format) + 1;
	
	char		*argument = malloc(argument_len);

	if (!argument)
		return NULL;
	snprintf(argument, argument_len, 
			"%s%s%s%s%s",
			json_format, model, json_format2, prompt, close_format);
	return argument;
}


static char	*exec(const char *url, char *argument)
{
	CURL *curl_handle;
	CURLcode res;
	char	*response = NULL;
	struct MemoryStruct chunk;

	if (!argument)
		return NULL;	
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
	const char	*end = "\",\"done\":";
	size_t		from = 0, to = 0, iterator = 0;

	for (size_t i = 0; (*s)[i] && (!from || !to); i++)
	{
		iterator = 0;
		while (!from
				&& start[iterator] && (*s)[i + iterator] 
				&& start[iterator] == (*s)[i + iterator]
				) iterator++;
		
		if (iterator == strlen(start))
			from = i + iterator;

		iterator = 0;
		while (!to
				&& end[iterator] && (*s)[i + iterator]
				&& end[iterator] == (*s)[i + iterator]) iterator++;
		if (iterator == strlen(end))
			to = i;
	}
	if (from >= to || !from || !to)
	{
		free(*s), *s = NULL;
        	return ;
	}

	char	*cleaned_up = malloc(to - from + 1);
    	if (!cleaned_up)
		return;

	strncpy(cleaned_up, *s + from, to - from);
	cleaned_up[to - from] = '\0';
	free(*s);
	*s = cleaned_up;
}


static void	display(char *s)
{
	if (!s)
		return ;
	printf("%s\n", s);
	fflush(stdout);
	return ; (void)s;
}

static char	*build_prompt(int ac, char **av)
{
	if (ac <= 2 || !av)
		return NULL;

	size_t	len = 0;
	char	*prompt = NULL;
	
	for (int i = 2; i < ac; i++)
	{
		if (!av[i])
			continue ;
		len += strlen(av[i]);
	}

	prompt = malloc(len + ac + 1);
	if (!prompt)
		return NULL;
	prompt[0] = '\0';

	for (int i = 2; i < ac; i++)
	{
		if (!av[i])
			continue;
		strncat(prompt, av[i], len + ac);
		strncat(prompt, " ", len + ac);
	}
	return prompt;
}

static void	clean_input(char **input)
{
	char *p = *input;
	
	while (*p) 
	{
		if (*p == '\\')// && (*(p + 1) != 'n' && *(p + 1) != 't'))
			*p = ' ';
		p++;
	}
}

char	*strreplace(const char *str, const char *old, const char *new)
{
    // Compter le nombre d'occurrences de 'old' dans 'str'
    int count = 0;
    const char *tmp = str;
    while ((tmp = strstr(tmp, old))) {
        count++;
        tmp += strlen(old);
    }

    // Allouer la mémoire pour la nouvelle chaîne
    size_t new_len = strlen(str) + count * (strlen(new) - strlen(old));
    char *result = malloc(new_len + 1);
    if (!result) return NULL;

    // Construire la nouvelle chaîne
    char *ptr = result;
    const char *start = str;
    while (count--) {
        const char *found = strstr(start, old);
        size_t len = found - start;
        strncpy(ptr, start, len);
        ptr += len;
        strcpy(ptr, new);
        ptr += strlen(new);
        start = found + strlen(old);
    }
    strcpy(ptr, start);

    return result;
}

int	main(int ac, char **av)	//	./ask_littletown [model] [prompt]
{
	char		*curl_argument = NULL;
	char		*prompt = NULL;
	char		*model = NULL;
	char		*response = NULL;
	const char	*url = "http://192.168.1.200:11434/api/generate";

//	for (int i = 2; i < ac; i++)
//		printf("AV[%d] : %s\n", i, av[i]); 
	if (ac <= 2) {
		return 0;
		prompt = get_next_line(0);
		if (prompt)
			prompt[strlen(prompt) - 1] = ' ';
		free(prompt);
		return 0;
	} else {
		prompt = build_prompt(ac, av);
		if (!prompt) return 0;
		if (!av[1]) return 0;
		model = strdup(av[1]);
		if (!model) return free(prompt), 0;
		
		clean_input(&prompt);
		
		char *new = strreplace(prompt, "\n", " ");
		free(prompt);
		prompt = new;
		new = strreplace(prompt, "\t", " ");
		free(prompt);
		prompt = new;
	}
	curl_argument = build_argument(prompt, model);
	response = exec(url, curl_argument);
	if (!response)
		return free(prompt), free(model), free(curl_argument), 0;
	clean_up_json(&response);
	display(response);
	free(response);
	free(prompt);
	free(model);
	if (curl_argument)
		free(curl_argument);
	return 1; 
}

