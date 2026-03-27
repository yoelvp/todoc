#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void print_lines(int width) {
    for (int i = 0; i < width; i++)
        printf("─");
    printf("\n");
}

static void add_notes(const char *path) {
    char buffer[200];
    FILE *f = fopen(path, "a");

    if (f == NULL) {
        perror("Error add new note");
        return;
    }

    printf("%p", fgets(buffer, sizeof(buffer), f));
    printf("Enter your note > ");
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        perror("fgets");
        fclose(f);
        return;
    }
    fprintf(f, "%s", buffer);
    fclose(f);
}

static void read_notes(const char *path) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1 || w.ws_col == 0)
        w.ws_col = 80;
    FILE *f = fopen(path, "r");
    char buffer[200];

    if (f == NULL) {
        perror("No notes yet\n");
        return;
    }

    print_lines(w.ws_col);
    while (fgets(buffer, sizeof(buffer), f) != NULL) {
        printf("%s", buffer);
    }
    print_lines(w.ws_col);

    fclose(f);
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

    if (snprintf(path, sizeof(path), "%s/%s", cwd, filename) >= sizeof(path)) {
        fprintf(stderr, "Path long\n");
        return 1;
    }

    do {
        printf("[a]dd - [r]ead [c]lear - [q]uit\n");
        printf("> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("\nEOF reciv, exit...");
            break;
        }

        if (buffer[0] == '\n')
            continue;
        option = buffer[0];

        switch (option) {
        case 'a':
            add_notes(path);
            break;
        case 'r':
            read_notes(path);
            break;
        case 'c':
            printf("\033[H\033[J");
            fflush(stdout);
            break;
        case 'q':
            break;
        default:
            printf("Invalid selected option.\n");
            printf("\n");
        }
    } while (option != 'q');

    return 0;
}
