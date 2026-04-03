#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void print_lines(int width) {
    for (int i = 0; i < width; i++)
        printf("─");
    printf("\n");
}

static void add_notes(const char *path) {
    char *buffer = NULL;
    size_t buflen = 0;
    FILE *f = fopen(path, "a+");

    if (f == NULL) {
        perror("Error adding a note.");
        fclose(f);
        return;
    }

    printf("Enter your note > ");
    fflush(stdout);
    if (!getline(&buffer, &buflen, stdin)) {
        perror("getline");
        fclose(f);
        return;
    }
    fprintf(f, "%s", buffer);
    fclose(f);
}

static void read_notes(const char *path) {
    FILE *f = fopen(path, "r");
    char buffer[200];
    int x = 1;
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0)
        w.ws_col = 80;

    if (f == NULL) {
        perror("No notes yet\n");
        return;
    }

    print_lines(w.ws_col);
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        printf("%4d  %s", x, buffer);
        x++;
    }
    print_lines(w.ws_col);

    fclose(f);
}

static void delete_note(char *path) {
    char **lines = NULL;
    int count = 0;
    char buffer[256];
    char id[8];
    FILE *f = fopen(path, "r");

    if (!f) {
        perror("File not found.");
        return;
    }

    while (fgets(buffer, sizeof(buffer), f)) {
        lines = realloc(lines, (count + 1) * sizeof(char *));
        lines[count] = malloc(strlen(buffer) + 1);
        strcpy(lines[count], buffer);
        count++;
    }
    fclose(f);

    if (count == 0) {
        printf("No notes to delete.\n");
        free(lines);
        return;
    }

    printf("Enter ID 1-%d > ", count);
    fflush(stdout);

    if (!fgets(id, sizeof(id), stdin)) {
        printf("Invalid input.\n");
        for (int i = 0; i < count; i++)
            free(lines[i]);
        free(lines);
        return;
    }

    int new_id = (int)strtol(id, NULL, 10);
    FILE *fw = fopen(path, "w");

    if (!fw) {
        perror("Error writting file");
        for (int i = 0; i < count; i++)
            free(lines[i]);
        free(lines);
        return;
    }

    for (int i = 0; i < count; i++) {
        if (i + 1 != new_id)
            fprintf(fw, "%s", lines[i]);
        free(lines[i]);
    }

    free(lines);
    fclose(fw);
    printf("Note %d deleted.\n", new_id);
}

int main() {
    char option;
    char buffer[200];
    char cwd[1024];
    char path[2048];
    const char *filename = "NOTES";

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return 1;
    }

    int written = snprintf(path, sizeof(path), "%s/%s", cwd, filename);
    if (written < 0 || (size_t)written >= sizeof(path)) {
        fprintf(stderr, "Path long\n");
        return 1;
    }

    do {
        printf("[a]dd - [r]ead - [d]elete - [c]lear - [q]uit\n");
        printf("> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("\nEOF reciv, exit...");
            break;
        }

        if (buffer[0] == '\n')
            continue;

        if (buffer[1] != '\n' && buffer[1] != '\0') {
            printf("Invalid option, please enter only one char.\n");
            continue;
        }

        option = buffer[0];

        switch (option) {
        case 'a':
            add_notes(path);
            break;
        case 'r':
            read_notes(path);
            break;
        case 'd':
            delete_note(path);
            break;
        case 'c':
            printf("\033[H\033[J");
            fflush(stdout);
            break;
        case 'q':
            break;
        default:
            printf("Invalid option.\n");
            printf("\n");
        }
    } while (option != 'q');

    return 0;
}
