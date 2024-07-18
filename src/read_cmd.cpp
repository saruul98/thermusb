#include "read_cmd.h"

void printOptions(WINDOW *win) {
    wprintw(win, "Usage: thermusb options\n");
    wprintw(win, "  -h --help     Display this usage information\n");
    wprintw(win, "  -d --debug    DEBUG mode (no hardware interface)\n");
}

void read_cmd(int argc, char **argv, WINDOW *win, struct str_def_usr_var *usr_var) {

    // char* arg_w = nullptr;

    int c;
    static struct option long_options[] = {
        {"help",          no_argument, 0, 'h'},
        {"debug",         no_argument, 0, 'd'},
        {0,0,0,0}
    };

    while (1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "hd", long_options, &option_index);

        if (c == -1)
          break;

        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
                wprintw(win, "options %s", long_options[option_index].name);
                if (optarg)
                    wprintw(win, " with arg %s", optarg);
                wprintw(win,"\n");
                refresh();
		        wrefresh(win);
                break;

            case 'h':
                printOptions(win);
                refresh();
		        wrefresh(win);
                std::this_thread::sleep_for(std::chrono::seconds(5));
                endwin();
                exit(0);
                break;

            case 'd':
                wprintw(win, "Starting DEBUG mode...\n");
                usr_var->debug_mode = true;
                break;

            default:
                endwin();
                abort();
            }
    }

    if (optind < argc) {
        wprintw(win,"non-option ARGV-elements:\n");
        while (optind < argc)
            wprintw(win,"%s ", argv[optind++]);
        refresh();
        wrefresh(win);
        putchar('\n');
    }

}
