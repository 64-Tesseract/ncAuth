#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "totp.c"


#define min(a, b) a < b ? a : b
#define max(a, b) a > b ? a : b


typedef struct { char name[128]; char secret[128]; char cur_code[7]; char next_code[7]; bool temp; } AuthCode;


void add_authcode (AuthCode* authcodes, int* counter, char* name, char* secret, bool temp) {
    // Create a temporary code with the supplied vars
    AuthCode new_code = {"", "", "------", "------", temp};
    strcpy(new_code.name, name);
    strcpy(new_code.secret, secret);
    
    // Add code to array
    authcodes[*counter] = new_code;
    *counter += 1;
}


int read_authcodes (AuthCode* authcodes, int* counter, char* path) {
    char line[256];
    FILE* file = fopen(path, "r");
    
    if (file != NULL) {
        while (fgets(line, 256, file)) {
            // Convert char[] to char*
            char* lineaddr = line;
            // Split the line by tabs, will segfault if incorrectly formatted
            char* name = strsep(&lineaddr, "\t");
            char* secret = strsep(&lineaddr, "\t");
            
            add_authcode(authcodes, counter, name, secret, FALSE);
        }
        
        fclose(file);
    } else {
        printf("No such secrets file\n");
    }
}


void calc_2fas (AuthCode* authcodes, int counter, uint64_t hotp) {
    for (int c = 0; c < counter; c++) {
        // Calculate each account's current & next code
        get_2fa_code(authcodes[c].secret, authcodes[c].cur_code, hotp);
        get_2fa_code(authcodes[c].secret, authcodes[c].next_code, hotp + 1);
    }
}


int calc_scroll (int select, int count, int height) {
    // If auth code list fits in window, put them up top
    if (count <= height) return 0;
    
    // By default, the 1st item (scroll) should be half the size of the view area
    // above the selection, so the selection is in the middle of the screen
    int scroll = select - height / 2;
    // Limit to 1st item being on top
    if (scroll < 0) scroll = 0;
    // Limit last item to being on bottom of view area
    if (scroll > count - height) scroll = count - height;
    return scroll;
}


void define_colour (int colour) {
    init_pair(1, COLOR_BLACK, colour);
    init_pair(2, colour, COLOR_BLACK);
}


void set_colour (int c) {
    // (int) -> (int, but with extra steps)
    switch (c) {
        default:
        case 0:
            define_colour(COLOR_WHITE);
            break;
        case 1:
            define_colour(COLOR_RED);
            break;
        case 2:
            define_colour(COLOR_GREEN);
            break;
        case 3:
            define_colour(COLOR_YELLOW);
            break;
        case 4:
            define_colour(COLOR_BLUE);
            break;
        case 5:
            define_colour(COLOR_MAGENTA);
            break;
        case 6:
            define_colour(COLOR_CYAN);
            break;
    }
}


void write_colour (int colour) {
    char path[256];
    // Finds file `colour` in exe's directory
    snprintf(path, 256, "%s/colour", getcwd(NULL, 256));
    FILE* file = fopen(path, "wb+");
    if (file != NULL) {
        // Colour is int & only needs 1 byte
        fputc(colour, file);
        fclose(file);
    } else {
        printf("Could not save colour\n");
        exit(1);
    }
}


int read_colour () {
    int c;
    char path[256];
    // Finds file `colour` in exe's directory
    snprintf(path, 256, "%s/colour", getcwd(NULL, 256));
    FILE* file = fopen(path, "rb");
    if (file != NULL) {
        // Colour is int & only needs 1 byte
        c = fgetc(file);
        fclose(file);
    } else {
        // If no colour file exists, make one
        c = 0;
        write_colour(0);
        printf("Could not load colour\n");
    }
    
    return c;
}


int main (int argc, char *argv[]) {
    int authcodec, clicodec = 0;
    AuthCode authcodev[128];
    struct timespec curtime;
    
    bool no_file = FALSE;
    bool changed_dir = FALSE;
    char file[256];
    snprintf(file, 256, "%s/secrets.tsv", getcwd(NULL, 256));
    bool print_mode = FALSE;
    
    
    // Literally half of the code is just argument parsing
    for (int a = 1; a < argc; a++) {
        if (strcmp(argv[a], "--help") == 0 || strcmp(argv[a], "-h") == 0) {
            printf("   ncAuth ~ Terminal TOTP Authenticator by 64_Tesseract\n");
            printf("--help, -h              Prints this list & exits\n");
            printf("--code [name] [code]    Adds a temporary TOTP code to generate this session\n");
            printf("--file [file]           Specifies a secret file, by default \"./secrets.tsv\"\n");
            printf("                          The format for each line is: NAME <tab> SECRET\n");
            printf("                          If file is \"none\", does not load a secrets file\n");
            printf("--print                 Prints all account names & generated codes instead of\n");
            printf("                          running in interactive mode\n");
            return 0;
            
        } else if (strcmp(argv[a], "--code") == 0) {
            if (argc <= a + 2) {
                printf("Missing a name and/or secret\n");
                exit(1);
            }
            
            add_authcode(authcodev, &clicodec, argv[a + 1], argv[a + 2], TRUE);
            a += 2;
            
        } else if (strcmp(argv[a], "--file") == 0) {
            if (changed_dir) {
                printf("Multiple secrets file paths\n");
                exit(7);
            }
            if (argc <= a + 1) {
                printf("Missing a file path\n");
                exit(1);
            }
            
            changed_dir = TRUE;
            char* new_file = argv[++a];
            
            // If file is "none", don't load a file
            // ~~rip to anyone who names their files after programming language keywords~~
            if (strcmp(new_file, "none") == 0)
                no_file = TRUE;
            else
                strcpy(file, new_file);
            
        } else if (strcmp(argv[a], "--print") == 0) {
            if (print_mode) {
                printf("Print mode already set\n");
                exit(7);
            }
            
            print_mode = TRUE;
            
        } else {
            printf("Unknown argument \"%s\", see --help\n", argv[a]);
            exit(1);
        }
    }
    
    authcodec = clicodec;
    
    if (!no_file)
        read_authcodes(authcodev, &authcodec, file);
    
    
    // Just calculate all codes once, print, & die
    if (print_mode) {
        clock_gettime(CLOCK_REALTIME, &curtime);
        calc_2fas(authcodev, authcodec, curtime.tv_sec / 30);
        
        for (int c = 0; c < authcodec; c++)
            printf("%s\t%s\n", authcodev[c].name, authcodev[c].cur_code);
        exit(0);
    }
    
    
    // 2nd half of code: ncurses GUI
    initscr();
    start_color();
    noecho();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    
    // Why don't I define these with the other vars? Too much scrolling
    int select, scroll, name_width, last_hotp = 0;
    int highlight, highlight_time = 0;
    bool view_next = FALSE;
    
    int colour = read_colour();
    set_colour(colour);
    
    bool gui_running = TRUE;
    while (gui_running) {
        usleep(0x10000);
        clear();
        clock_gettime(CLOCK_REALTIME, &curtime);
        
        // Check if time's passed 30 seconds - if the new 30 second interval is different to the last
        int new_hotp = curtime.tv_sec / 30;
        if (new_hotp != last_hotp) {
            // Calculate new auth codes
            calc_2fas(authcodev, authcodec, curtime.tv_sec / 30);
            last_hotp = new_hotp;
            // If you're typing the next (this new) code & it updates on you, display this (last next) code automatically
            view_next = FALSE;
        }
        
        // Auth code takes 7 chars (with space), plus 2 padding in between & 2 padding outside
        name_width = COLS - 11;
        
        scroll = calc_scroll(select, authcodec, LINES - 1);
            
        
        char script_sh[512];
        
        switch (getch()) {
            // Change selection
            case 'k':
                if (select != 0)
                    scroll = calc_scroll(--select, authcodec, LINES - 1);
                break;
            case 'j':
                if (select != authcodec - 1)
                    scroll = calc_scroll(++select, authcodec, LINES - 1);
                break;
            
            // Toggle next view
            case 'n':
                view_next = !view_next;
                break;
            
            // Cycle colour
            case 'c':
                colour = (colour + 1) % 7;
                set_colour(colour);
                break;
            
            // Run scripts to edit the secrets file or do something else
            case 's':
                snprintf(script_sh, 512, "%s/script.sh \"%s\" \"%s\"", getcwd(NULL, 256), authcodev[select].name, view_next ? authcodev[select].next_code : authcodev[select].cur_code);
                
                // If no error in script, highlight selected auth code, otherwise pause to read errors
                highlight = select;
                if (system(script_sh) == 0)
                    highlight_time = 0x40;
                else
                    usleep(0x200000);
                break;
            case 'e':
                // Can't edit the file if there is no file
                if (no_file)
                    break;
                
                snprintf(script_sh, 512, "%s/edit.sh \"%s\"", getcwd(NULL, 256), file);
                
                attron(A_BLINK);
                mvprintw(0, 0, "Waiting for editor\n");
                attroff(A_BLINK);
                refresh();
                clear();
                
                // If no error in script, wait for editor to finish & reload auth codes, otherwise pause to read errors
                if (system(script_sh) == 0) {
                    select = 0;
                    scroll = 0;
                    // If codes defined from args, rewrite auth codes only from after them
                    authcodec = clicodec;
                    highlight_time = 0;
                    read_authcodes(authcodev, &authcodec, file);
                    calc_2fas(authcodev, authcodec, curtime.tv_sec / 30);
                } else {
                    usleep(0x200000);
                }
                break;
            
            // die lol
            case 'q':
                gui_running = FALSE;
                break;
        }
        
        
        if (authcodec != 0) {
            // Run only in visible codes, dependent on scroll & list count
            for (int a = 0; a < (min(LINES - 1, authcodec - scroll)); a++) {
                int aa = a + scroll;
                
                // Copy account name to temp var
                char name[256];
                strcpy(name, authcodev[aa].name);
                
                // If name too long, add "..." & define new string ending
                if (strlen(name) > name_width) {
                    name[name_width - 3] = '.';
                    name[name_width - 2] = '.';
                    name[name_width - 1] = '.';
                    name[name_width] = '\0';
                }
                
                // Create new var for code, format to put space in middle
                char code[8];
                code[3] = ' ';
                code[7] = '\0';
                strncpy(code, view_next ? authcodev[aa].next_code : authcodev[aa].cur_code, 3);
                strncpy(&code[4], &(view_next ? authcodev[aa].next_code : authcodev[aa].cur_code)[3], 3);
                
                // Put name & code together
                char text[512];
                snprintf(text, 512, "%c%-*s  %s ", authcodev[aa].temp ? '~' : ' ', name_width, name, code);
                
                if (aa == highlight && highlight_time != 0) {
                    attron(A_BLINK);
                    highlight_time--;
                }
                
                if (aa == select) {
                    // Calculate progress bar based off time
                    int pbar = ((curtime.tv_sec * 100 + curtime.tv_nsec / 10000000) % 3000) * COLS / 3000;
                    
                    // Split left & right sides of text within progress bar
                    char left[512];
                    strncpy(left, text, pbar + 1);
                    left[pbar + 1] = '\0';
                    
                    // Colour left side & uncolour right
                    attrset(view_next ? COLOR_PAIR(2) : COLOR_PAIR(1));
                    mvprintw(a, 0, "%s", left);
                    attrset(view_next ? COLOR_PAIR(1) : COLOR_PAIR(2));
                    mvprintw(a, pbar + 1, "%s", &text[pbar + 1]);
                } else {
                    // If not selected, just print normally
                    attrset(COLOR_PAIR(0));
                    mvprintw(a, 0, "%s", text);
                }
                
                attroff(A_BLINK);
            }
            
            attroff(A_ITALIC);
        } else {
            // If no codes, show error in middle of screen
            attron(A_BLINK);
            mvprintw((LINES - 1) / 2, COLS / 2 - 6, "- No codes -");
            attroff(A_BLINK);
        }
        
        attrset(COLOR_PAIR(2));
        attron(A_BOLD);
        // Show controls at bottom of screen, just the hotkeys if terminal's too narrow
        if (COLS >= 39) {
            mvprintw(LINES - 1, 0, "[KJ]^/v");
            mvprintw(LINES - 1, COLS / 4 - 1, "[C]olour");
            if (highlight_time != 0) attron(A_BLINK);
            mvprintw(LINES - 1, COLS / 2 - 2, "[S]cript");
            attroff(A_BLINK);
            mvprintw(LINES - 1, COLS * 3 / 4 - 3, "[E]dit");
            if (view_next) attron(A_BLINK);
            mvprintw(LINES - 1, COLS - 6, "[N]ext");
            attroff(A_BLINK);
        } else {
            mvprintw(LINES - 1, 0, "[KJ]");
            mvprintw(LINES - 1, COLS / 4, "[C]");
            if (highlight_time != 0) attron(A_BLINK);
            mvprintw(LINES - 1, COLS / 2 - 1, "[S]");
            attroff(A_BLINK);
            mvprintw(LINES - 1, COLS * 3 / 4 - 2, "[E]");
            if (view_next) attron(A_BLINK);
            mvprintw(LINES - 1, COLS - 3, "[N]");
            attroff(A_BLINK);
        }
        attroff(A_BOLD);
        attroff(COLOR_PAIR(2));
        
        refresh();
    }
    
    // Save colour
    write_colour(colour);
    endwin();
}

