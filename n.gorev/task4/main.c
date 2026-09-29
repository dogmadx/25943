#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 1024

struct node {
    char *str;
    struct node *next;
};

int main(void)
{
    char buf[MAXLEN];
    struct node *head = NULL, *tail = NULL, *p, *next;

    while (fgets(buf, sizeof buf, stdin) != NULL && buf[0] != '.') {
        size_t len = strlen(buf);

        struct node *n = malloc(sizeof *n);
        n->str = malloc(len + 1);
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
