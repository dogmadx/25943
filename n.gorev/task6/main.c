#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

int fd;

void timeout(int sig) {
    (void)sig;
    char buf[256];
    ssize_t r;

    write(1, "\nвремя вышло\n", sizeof("\nвремя вышло\n") - 1);
    lseek(fd, 0L, 0);
    while ((r = read(fd, buf, sizeof(buf))) > 0)
        write(1, buf, r);

    _exit(0);
}

int main(void) {
    char filename[256];

    printf("имя файла: ");
    if (scanf("%255s", filename) != 1)
        return 1;

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("ошибка чтения");
        return 1;
    }

    size_t capacity = 16;
    size_t line_count = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));

    off_t line_start = 0;
    char ch;

    while (read(fd, &ch, 1) > 0) {
        if (ch == '\n') {
            off_t pos = lseek(fd, 0L, 1);

            if (line_count == capacity) {
                capacity *= 2;
                LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));

                table = tmp;
            }

            table[line_count].offset = line_start;
            table[line_count].length = (size_t)(pos - line_start);
            line_count++;

            line_start = pos;
        }
    }

    off_t file_size = lseek(fd, 0L, 1);
    if (file_size > line_start) {
        if (line_count == capacity) {
            capacity += 1;
            LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));

            table = tmp;
        }
        table[line_count].offset = line_start;
        table[line_count].length = (size_t)(file_size - line_start);
        line_count++;
    }

    printf("\n%-8s %-8s %-8s\n", "строка", "отступ", "длина");
    for (size_t i = 0; i < line_count; i++)
        printf("%-8zu %-8lld %-8zu\n", i + 1, (long long)table[i].offset, table[i].length);
    printf("\n");

    signal(SIGALRM, timeout);

    int n;
    while (1) {
        printf("введите номер строки (5 секунд): ");
        fflush(stdout);

        alarm(5);                
        int ok = scanf("%d", &n);
        alarm(0);                

        if (ok != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                continue;
            if (c == EOF)
                break;
            printf("плохой ввод\n");
            continue;
        }

        if (n == 0)
            break;

        if (n < 0 || (size_t)n > line_count) {
            printf("нет такой строки\n");
            continue;
        }

        LineInfo *li = &table[n - 1];
        char *buf = malloc(li->length + 1);

        lseek(fd, li->offset, 0);
        ssize_t r = read(fd, buf, li->length);
        buf[r] = '\0';

        printf("%s", buf);
        if (r == 0 || buf[r - 1] != '\n')
            printf("\n");

        free(buf);
    }

    free(table);
    close(fd);
    return 0;
}