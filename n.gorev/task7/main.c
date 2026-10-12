#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    off_t offset;
    size_t length;
} LineInfo;

char *map;
off_t size;

void timeout(int sig) {
    (void)sig;
    printf("\nвремя вышло\n%.*s\n", (int)size, map);
    exit(0);
}

int main(void) {
    char filename[256];

    printf("имя файла: ");
    if (scanf("%255s", filename) != 1)
        return 1;

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("ошибка чтения");
        return 1;
    }

    struct stat st;
    fstat(fd, &st);
    size = st.st_size;
    if (size == 0) {
        printf("файл пустой\n");
        return 0;
    }

    map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("ошибка mmap");
        return 1;
    }
    close(fd);

    size_t capacity = 16;
    size_t line_count = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));

    off_t line_start = 0;

    for (off_t i = 0; i < size; i++) {
        if (map[i] == '\n') {
            off_t pos = i + 1;

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

    if (size > line_start) {
        if (line_count == capacity) {
            capacity += 1;
            LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));

            table = tmp;
        }
        table[line_count].offset = line_start;
        table[line_count].length = (size_t)(size - line_start);
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

        printf("%.*s", (int)li->length, map + li->offset);
        if (map[li->offset + li->length - 1] != '\n')
            printf("\n");
    }

    free(table);
    munmap(map, size);
    return 0;
}