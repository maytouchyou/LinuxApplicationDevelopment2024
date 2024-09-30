#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define KEY_ESC 27


int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage : %s <file_to_open>\n", argv[0]);
        return 0;
    }

    struct stat sb;
    if (stat(argv[1], &sb) == -1) {
        printf("Cannot access to file : %s \n", argv[1]);
        return 0;
    }

    FILE *file = fopen(argv[1], "r");
    char *buf = (char *) calloc(sb.st_size, 1);

    int size=0;
    char **lines = NULL;

    while (!feof(file))
    {
        fgets(buf, sb.st_size, file);
        if (!lines)
            lines = malloc((size = 1) * sizeof(char *));
        else
            lines = realloc(lines, ++size * sizeof(char *));

        lines[size-1] = strdup(buf);
    }
    fclose(file);

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    refresh();

    printw("File: %s, len: %d", argv[1], sb.st_size);

    int startx  = 4, starty  = 3,
        scrollx = 0, scrolly = 0;

    int height = (LINES - starty) / 2,
        width  = (COLS  - startx) / 2;

    WINDOW *box_win  = newwin(height+1, width+1, starty, startx);

    box(box_win, 0, 0);
    wrefresh(box_win);

    int num_len = sprintf(buf, "%d", size);
    sprintf(buf, "%%%dd: %%.%ds", num_len, width-num_len-3);
    char *mask = strdup(buf);

    int ch;
    while((ch = getch()) != KEY_ESC)
    {
        if ((ch == KEY_LEFT) && (scrollx > 0)) {
            scrollx--;
        }
        else if (ch == KEY_RIGHT) {
            scrollx++;
        }
        else if ((ch == KEY_UP) && (scrolly > 0)) {
            scrolly--;
        }
        else if (ch == KEY_DOWN) {
            scrolly++;
        }
        wclear(box_win);

        int l;
        for (l = 0; l < height; l++)
            mvwprintw(box_win, l, 1, mask, l+scrolly, (l+scrolly < size && scrollx < strlen(lines[l+scrolly])) ? lines[l+scrolly]+scrollx : "");
            //mvwprintw(box_win, l+1, 1, "%d", strlen(lines[l]));
        box(box_win, 0, 0);
        wrefresh(box_win);

    }

    endwin();

    return 0;
}
