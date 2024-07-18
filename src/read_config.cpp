#include "read_config.h"


void read_conf_file(struct str_def_usr_var *usr_var, const char* conf_file, WINDOW *win)
{
    config_t cfg;
    const char *str = NULL;
    int buf_int;
    double buf_double;
    char dummy = 1;

    wprintw(win, "Processing configuration file: %s\n", conf_file);
    refresh();
    wrefresh(win);

    config_init(&cfg);

    if (!config_read_file(&cfg, conf_file)) {
       wprintw(win, "%s:%d - %s\n", &dummy, config_error_line(&cfg), config_error_text(&cfg));
       refresh();
       wrefresh(win);
       config_destroy(&cfg);
       return;
    }
    
    if(config_lookup_string(&cfg, "comment", &str)) {
        // usr_var->comment = (char *) str;
        sprintf(usr_var->comment, str);
        wprintw(win, "    Comment: %s\n", usr_var->comment);      
        refresh();
        wrefresh(win);  
    }
    else {
        wprintw(win, "comment missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }

    if(config_lookup_int(&cfg, "output", &buf_int)) {
        usr_var->output = (int) buf_int;
    }
    else {
        wprintw(win, "output missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }

    if(config_lookup_float(&cfg, "R2", &buf_double)) {
        usr_var->R2 = (double) buf_double;
        wprintw(win, "    R2: %f\n", usr_var->R2);
        refresh();
        wrefresh(win);
    }
    else {
        wprintw(win, "Error: R2 missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }

    if(config_lookup_float(&cfg, "R3", &buf_double)) {
        usr_var->R3 = (double) buf_double;
        wprintw(win, "    R3: %f\n", usr_var->R3);
        refresh();
        wrefresh(win);
    }
    else {
        wprintw(win, "Error: R3 missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }

    if(config_lookup_float(&cfg, "R4", &buf_double)) {
        usr_var->R4 = (double) buf_double;
        wprintw(win, "    R4: %f\n", usr_var->R4);
        refresh();
        wrefresh(win);
    }
    else {
        wprintw(win, "Error: R4 missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }

    if(config_lookup_float(&cfg, "vs", &buf_double)) {
        usr_var->vs = (double) buf_double;
        wprintw(win, "    vs: %f\n", usr_var->vs);
        refresh();
        wrefresh(win);
    }
    else {
        wprintw(win, "Error: vs missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }

    if(config_lookup_float(&cfg, "g", &buf_double)) {
        usr_var->g = (double) buf_double;
        wprintw(win, "    g: %f\n", usr_var->g);
        refresh();
        wrefresh(win);
    }
    else {
        wprintw(win, "Error: g missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }

    if(config_lookup_int(&cfg, "thermistor_choice", &buf_int)) {
        usr_var->thermistor_choice = (int) buf_int;
        if (usr_var->thermistor_choice == 0)
            sprintf(usr_var->thermistor_name, "PT10000");
        else
            sprintf(usr_var->thermistor_name, "SEMI833ET");
        wprintw(win, "    thermistor_choice: %d\n", usr_var->thermistor_choice);
        refresh();
        wrefresh(win);
    }
    else {
        wprintw(win, "Error: thermistor_choice missing in %s\n", conf_file);
        refresh();
        wrefresh(win);
        exit (0);
    }


}
