#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define KEY_ESC 27


int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage : %s <file_to_open>\n", argv[0]);
        return EXIT_FAILURE;
    }


    // File routines
    struct stat sb;
    if (stat(argv[1], &sb) == -1) {
        fprintf(stderr, "Cannot access to file : %s \n", argv[1]);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Cannot open file : %s \n", argv[1]);
        return EXIT_FAILURE;
    }

    char *buf;
    if ( (buf = (char *) calloc(sb.st_size, 1)) == NULL ){
        fprintf(stderr, "Error: memory alloc");
        return EXIT_FAILURE;
    }

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
            return EXIT_FAILURE;
        }

        if (!(text[lines - 1] = strdup(buf))) {
            fprintf(stderr, "Error: memory alloc");
            return EXIT_FAILURE;
        }
    }
    fclose(file);


    // Ncurses engine initialization
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    refresh();

    printw("File: %s, len: %d", argv[1], sb.st_size);


    // Default scrollable window geometry
    int win_left  = 4, win_top  = 3,
        horiz_scroll = 0, vert_scroll = 0;

    int win_lines = (LINES - win_top) / 2,
        win_cols  = (COLS  - win_left) / 2;

    WINDOW *win  = newwin(win_lines + 1, win_cols + 1, win_top, win_left);

    box(win, 0, 0);
    wrefresh(win);


    // Create file description above window
    int num_len = sprintf(buf, "%d", lines);

    sprintf(buf, "%%%dd: %%.%ds", num_len, (win_cols - num_len - 3) );
    char *mask = strdup(buf);


    // User input process
    int ch;
    while((ch = getch()) != KEY_ESC)
    {
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
        wclear(win);

        for (int l = 0; l < win_lines; l++)
        {
            int new_line = l + vert_scroll;

            char *display_text = "";
            if (new_line < lines)
            {
                if (horiz_scroll < strlen(text[new_line])) {
                    display_text = text[new_line] + horiz_scroll;
                }
            }

            mvwprintw(win, l, 1, mask, new_line, display_text);
        }

        box(win, 0, 0);
        wrefresh(win);
    }

    endwin();


    free(buf);
    free(mask);

    for (int i = 0; i < lines; i++) {
        free(text[i]);
    }

    return EXIT_SUCCESS;
}
