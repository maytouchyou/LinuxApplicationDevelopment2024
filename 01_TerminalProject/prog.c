#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define KEY_ESC 27


/* Default scrollable window geometry */

struct win_params_t {
    WINDOW *win;

    int left, top;
    int cols, rows;
    int nc_cols, nc_rows; // ncurses cols & rows
};

struct file_params_t {
    int fsize, flines;
    char *fname;
    char **ftext;
};

bool
read_file(struct file_params_t *fp)
{
    struct stat sb;
    if (stat(fp->fname, &sb) == -1) {
        fprintf(stderr, "Cannot access to file : %s \n", fp->fname);
        return false;
    }

    FILE *file = fopen(fp->fname, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file : %s \n", fp->fname);
        return false;
    }

    char *buf;
    if ( (buf = (char *) calloc(sb.st_size, 1)) == NULL ){
        fprintf(stderr, "Error: memory alloc");
        return false;
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
            exit(EXIT_FAILURE);
        }

        if (!(text[lines - 1] = strdup(buf))) {
            fprintf(stderr, "Error: memory alloc");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);
    free(buf);

    if (fp) {
        fp->ftext = text;
        fp->fsize = sb.st_size;
        fp->flines = lines;
    }
    return true;
}

int
init_ncurses(struct win_params_t *wp)
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    refresh();

    if (wp) {
        wp->nc_rows = LINES;
        wp->nc_cols = COLS;

        wp->rows = (LINES - wp->top) / 2;
        wp->cols  = (COLS  - wp->left) / 2;
    }
}

int
draw_window(struct win_params_t *wp,
            const struct file_params_t *fp)
{
    wp->win = newwin(wp->rows + 1, wp->cols + 1, wp->top, wp->left);

    box(wp->win, 0, 0);
    wrefresh(wp->win);
    printw("File: %s, len: %d", fp->fname, fp->fsize); // Print file info above window
}

void
keypress_loop(const struct win_params_t *wp,
              const struct file_params_t *fp)
{
    char line_prefix[32];
    int num_len = snprintf(line_prefix, 32, "%d", fp->flines);

    char mask[256];
    sprintf( mask, "%%%dd: %%.%ds", num_len, ( wp->cols - num_len - 3 ));

    int horiz_scroll = 0, vert_scroll = 0;

    // Process keyboard events
    int ch;
    do {
        wclear(wp->win);

        for (int l = 0; l < fp->flines; l++)
        {
            // Print empty line when we scroll text outside the window
            char *display_text = "";

            int new_line = l + vert_scroll;
            if (new_line < fp->flines)
            {
                if (horiz_scroll < strlen( fp->ftext[new_line] )) {
                    display_text = fp->ftext[new_line] + horiz_scroll;
                }
            }
            mvwprintw( wp->win, l, 1, mask, new_line, display_text );
        }

        box( wp->win, 0, 0 );
        wrefresh( wp->win );

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

void free_memory(struct file_params_t *fp)
{
    for (int i = 0; i < fp->flines; i++) {
        free(fp->ftext[i]);
    }
    free(fp->ftext);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage : %s <file_to_open>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct win_params_t wp = {.left = 3, .top  = 4};
    struct file_params_t fp = {.fname = argv[1]};

    if (read_file(&fp))
    {
        init_ncurses(&wp);
        draw_window(&wp, &fp);
        keypress_loop(&wp, &fp);
        free_memory(&fp);
    }

    return EXIT_SUCCESS;
}
