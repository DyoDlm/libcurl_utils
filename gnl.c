#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 256
void	free_data(char **s);

size_t  ft_strlen(const char *s)
{
        size_t  i;

        i = 0;
        if (s[i] == 0 || !s)
                return (0);
        while (s[i])
                i++;
        return (i);
}

char    *ft_strchr(const char *s, int c)
{
        if (!s)
                return (NULL);
        while (*s != (char)c)
        {
                if (!*s)
                        return (NULL);
                s++;
        }
        return ((char *)s);
}

size_t  ft_strlcpy(char *dst, const char *src, size_t size)
{
        size_t  srclen;
        size_t  j;

        srclen = 0;
        j = 0;
        while (src[srclen])
                srclen++;
        if (size == 0)
                return (srclen);
        while (src[j] && j < size - 1)
        {
                dst[j] = src[j];
                j++;
        }
        dst[j] = '\0';
        return (srclen);
}

size_t  ft_strlcat(char *dst, const char *src, size_t size)
{
        size_t  i;
        size_t  srclen;
        size_t  destlen;

        destlen = 0;
        srclen = 0;
        i = 0;
        while (dst[destlen] && destlen < size)
                destlen++;
        while (src[srclen])
                srclen++;
        if (destlen >= size)
                return (size + srclen);
        while (src[i] && size - 1 > destlen + i)
        {
                dst[destlen + i] = src[i];
                i++;
        }
        dst[destlen + i] = '\0';
        return (destlen + srclen);
}

char    *ft_strjoin(char *s1, const char *s2)
{
        size_t  len1;
        size_t  len2;
        char    *result;

        if (!s2)
                return (s1);
        len1 = 0;
        if (s1)
                len1 = ft_strlen(s1);
        len2 = ft_strlen(s2);
        result = malloc(len1 + len2 + 1);
        if (!result)
        {
                free_data(&s1);
                return (NULL);
        }
        if (s1)
                ft_strlcpy(result, s1, len1 + 1);
        else
                result[0] = '\0';
        ft_strlcat(result, s2, len1 + len2 + 1);
        free(s1);
        return (result);
}

void    free_data(char **s)
{
        if (s && *s)
        {
                free(*s);
                *s = NULL;
        }
}

static char     *id_next(char **stack, char *next_line)
{
        char    *line;
        char    *oflow;
        size_t  len;

        len = next_line - *stack + 1;
        line = malloc(len + 1);
        if (!line)
                return (NULL);
        ft_strlcpy(line, *stack, len + 1);
        oflow = malloc(ft_strlen(next_line + 1) + 1);
        if (!oflow)
                return (free(line), NULL);
        ft_strlcpy(oflow, next_line + 1, ft_strlen(next_line + 1) + 1);
        free_data(stack);
        *stack = oflow;
        return (line);
}

static char     *extract_line(char **stack)
{
        char    *next_line;
        char    *line;

        next_line = ft_strchr(*stack, '\n');
        line = NULL;
        if (next_line)
                return (id_next(stack, next_line));
        else
        {
                line = *stack;
                *stack = NULL;
        }
        return (line);
}

static int      update_stack(int fd, char **stack, char *buffer)
{
        int     bcount;

        while (!ft_strchr(*stack, '\n'))
        {
                bcount = read(fd, buffer, BUFFER_SIZE);
                if (bcount < 0)
                        return (-1);
                if (bcount == 0)
                        break ;
                buffer[bcount] = '\0';
                *stack = ft_strjoin(*stack, buffer);
                if (!*stack)
                        return (-1);
        }
        return (0);
}


char    *get_next_line(int fd)
{
        static char     *stack = NULL;
        char            *line;
        char            *buffer;

        line = NULL;
        buffer = malloc((BUFFER_SIZE + 1) * sizeof(char));
        if (!buffer)
                return (NULL);
        if (fd < 0 || BUFFER_SIZE <= 0)
                return (free(buffer), NULL);
        if (update_stack(fd, &stack, buffer) < 0)
                return (free_data(&stack), free(buffer), NULL);
        if (!stack || *stack == '\0')
                return (free_data(&stack), free(buffer), NULL);
        line = extract_line(&stack);
        free(buffer);
        return (line);
}
