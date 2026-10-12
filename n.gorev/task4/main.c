#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 1024

struct node {
    char *str;
    struct node *next;
};

static void strip_ctrl(char *s)
{
    unsigned char *r = (unsigned char *)s;
    unsigned char *w = r;

    while (*r) {
        if (*r == 0x1B) {                       /* ESC */
            r++;
            if (*r == '[') {                    /* CSI: ESC [ параметры финальный_байт */
                r++;
                while (*r >= 0x20 && *r <= 0x3F) /* параметры и промежуточные байты */
                    r++;
                if (*r >= 0x40 && *r <= 0x7E)   /* финальный байт (A, B, C, D, ~ ...) */
                    r++;
            } else if (*r == 'O') {             /* SS3: ESC O x (стрелки/F1-F4) */
                r++;
                if (*r >= 0x40 && *r <= 0x7E)
                    r++;
            } else if (*r >= 0x20 && *r <= 0x7E) {
                r++;                            /* ESC + один символ (Alt+клавиша) */
            }
        } else if ((*r < 0x20 && *r != '\n' && *r != '\t') || *r == 0x7F) {
            r++;                                /* прочие служебные символы */
        } else {
            *w++ = *r++;
        }
    }
    *w = '\0';
}

int main(void)
{
    char buf[MAXLEN];
    struct node *head = NULL, *tail = NULL, *p, *next;

    while (fgets(buf, sizeof buf, stdin) != NULL) {
        strip_ctrl(buf);

        if (buf[0] == '.')          /* проверяем уже очищенную строку */
            break;
        if (buf[0] == '\0')         /* от строки ничего не осталось */
            continue;

        size_t len = strlen(buf);

        struct node *n = malloc(sizeof *n);
        if (n == NULL) {
            perror("malloc");
            break;
        }
        n->str = malloc(len + 1);
        if (n->str == NULL) {
            perror("malloc");
            free(n);
            break;
        }
        strcpy(n->str, buf);
        n->next = NULL;

        if (tail == NULL)
            head = n;
        else
            tail->next = n;
        tail = n;
    }

    for (p = head; p != NULL; p = p->next)
        fputs(p->str, stdout);

    for (p = head; p != NULL; p = next) {
        next = p->next;
        free(p->str);
        free(p);
    }
    return 0;
}