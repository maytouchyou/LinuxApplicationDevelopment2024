#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define KEY_ESC 27


/* Default scrollable window geometry */

WINDOW *win;

int win_left, win_top;
int win_lines, win_cols;

int lines_count;
char **text_from_file;


char** read_file(const char *name, int *size)
{
    struct stat sb;
    if (stat(name, &sb) == -1) {
        fprintf(stderr, "Cannot access to file : %s \n", name);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(name, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file : %s \n", name);
        exit(EXIT_FAILURE);
    }

    char *buf;
    if ( (buf = (char *) calloc(sb.st_size, 1)) == NULL ){
        fprintf(stderr, "Error: memory alloc");
        exit(EXIT_FAILURE);
    }

    if (size) *size = sb.st_size;

    int lines = 0;
    char **text = NULL;

    while (fgets(buf, sb.st_size, file))
    {
        if (!text) {
            text = malloc((lines = 1) * sizeof(char *));
        }
        else {
            text = realloc(text, ++lines * sizeof(char *));
        }

        if (!text) {
            fprintf(stderr, "Error: memory alloc");
            exit(EXIT_FAILURE);
        }

        if (!(text[lines - 1] = strdup(buf))) {
            fprintf(stderr, "Error: memory alloc");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    free(buf);

    text_from_file = text;
    lines_count = lines;

    return text;
}

int init_ncurses()
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    refresh();

    win_left  = 4, win_top  = 3;
    win_lines = (LINES - win_top) / 2;
    win_cols  = (COLS  - win_left) / 2;
}

WINDOW* draw_window(const char *fname, int size)
{
    win  = newwin(win_lines + 1, win_cols + 1, win_top, win_left);
    box(win, 0, 0);
    wrefresh(win);

    // Print file description above window
    printw("File: %s, len: %d", fname, size);

    return win;
}

void keypress_loop()
{
    char line_prefix[32];
    int num_len = sprintf(line_prefix, "%d", lines_count);

    char mask[256];
    sprintf(mask, "%%%dd: %%.%ds", num_len, (win_cols - num_len - 3) );

    int horiz_scroll = 0, vert_scroll = 0;

    // User input process
    int ch;
    do {
        wclear(win);

        for (int l = 0; l < win_lines; l++)
        {
            // Print empty line when we scroll text outside the window
            char *display_text = "";

            int new_line = l + vert_scroll;
            if (new_line < lines_count)
            {
                if (horiz_scroll < strlen( text_from_file[new_line]) ) {
                    display_text = text_from_file[new_line] + horiz_scroll;
                }
            }
            mvwprintw(win, l, 1, mask, new_line, display_text);
        }

        box(win, 0, 0);
        wrefresh(win);

        ch = getch();
        if ((ch == KEY_LEFT) && (horiz_scroll > 0)) {
            horiz_scroll--;
        }
        else if (ch == KEY_RIGHT) {
            horiz_scroll++;
        }
        else if ((ch == KEY_UP) && (vert_scroll > 0)) {
            vert_scroll--;
        }
        else if (ch == KEY_DOWN) {
            vert_scroll++;
        }

    } while(ch != KEY_ESC);

    endwin();
}

void free_memory()
{
    for (int i = 0; i < lines_count; i++) {
        free(text_from_file[i]);
    }
    free(text_from_file);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage : %s <file_to_open>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int size;
    if (text_from_file = read_file(argv[1], &size))
    {
        init_ncurses();
        win = draw_window(argv[1], size);

        keypress_loop();
        free_memory();
    }

    return EXIT_SUCCESS;
}
