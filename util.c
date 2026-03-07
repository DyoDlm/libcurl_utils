#include "util.h"

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    // Vérifie que contents et mem ne sont pas NULL
    if (!contents || !mem) {
        return 0;
    }

    // Réalloue la mémoire avec une marge de sécurité
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Erreur : realloc a échoué (taille demandée : %zu)\n", mem->size + realsize + 1);
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Termine la chaîne

    return realsize;
}

/*size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    printf("error: not enough memory\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}*/
