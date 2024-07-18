#include "src/main.h"
#include "src/read_config.h"
#include "src/read_cmd.h"
#include "src/aux_functions.h"
#include "src/ftdi.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sstream>
#include <vector>

#define DEV_VENDOR 0x0403                   // USB device vendor ID
#define DEV_PRODUCT 0x6001                  // USB device product ID
#define DEV_DESCRIPTION "FT245R USB FIFO"   // USB device description
#define DEV_DESCRIPTION_SIZE 16             // Number of characters in DEV_DESCRIPTION
#define CONF_FILE "conf.cfg"                // Configuration file

#ifdef __WIN32__
#define sleep(x) Sleep(x)
#else
#include <sys/time.h>
#include <sys/stat.h>
#endif

void wprint(WINDOW* &win, char const *message) {
    wprintw(win,"%s", message);
    refresh();
    wrefresh(win);
}


int main(int argc, char** argv) {
    printf("\n\nTHERMUSB PROGRAM START !!!\n");
    printf("Starting program interface...\n");

    /* Resource allocation */
    struct str_def_usr_var* usr_var = NULL;
    usr_var = (struct str_def_usr_var*)malloc(sizeof(struct str_def_usr_var));
    sprintf(usr_var->comment, "Message goes here");
    usr_var->R2 = 20500.0;
    usr_var->R3 = 10000.0;
    usr_var->R4 = 10000.0;
    usr_var->vs = 5.0;
    usr_var->thermistor_choice = 0;
    WINDOW *window_stdout;
    WINDOW *window_readout;
    WINDOW *window_header;
    FILE *v_output, *t_output;
    // CLOCK clock;
    std::vector<ftdi_context*> ftdi_cntx;
    struct ftdi_context *ftdic;
    struct ftdi_device_list *devlist, *curdev;
    std::vector<int> ftdi_error;
    std::vector<unsigned char*> packet_bytes;
    int ret, rsize;
    char manufacturer[128], description[128];
    unsigned int i, n, ndev;
    char c; 
    bool run = 1;
    time_t startTime;
    long double sampleTime, firstSampleTime;
    char data_storage_folder [] = "readout/%s/";
    char data_storage_path [sizeof data_storage_folder+30];
    char time_buf [20];
    char file_path[50], file_path2[50];
    time_t rawtime;
    struct tm *timeinfo;
    // global variables

    /* ncurses window */
    // does not work on WIN32
    // to-do: implement cross-platform graphical interface
    initscr();
    start_color();
    window_readout  = newwin(20, 160, 21, 0);
    window_stdout = newwin(20, 160, 0, 0);
    window_header = newwin(1, 160, 20, 0);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    bkgd(COLOR_PAIR(1));
    wbkgd(window_stdout, COLOR_PAIR(2));
    wbkgd(window_readout, COLOR_PAIR(2));
    wbkgd(window_header, COLOR_PAIR(2));
    scrollok(window_readout, TRUE);
    scrollok(window_stdout, TRUE);

    wprint(window_stdout, "Multichannel FPGA Temperature Readout via USB\n");

    /* Processing commands */
    read_cmd(argc, argv, window_stdout, usr_var);

    /* Processing configuration file */
    read_conf_file(usr_var, CONF_FILE, window_stdout);

    /* Discovering all FTDI devices */
    wprint(window_stdout, "Initializing ftdi context for ftdi_usb_find_all...\n");
    if ((ftdic = ftdi_new()) == 0) wprint(window_stdout, "Error: ftdi_new failed\n");
    wprint(window_stdout, "Discovering FTDI devices...\n");

    if ((ret = ftdi_usb_find_all(ftdic, &devlist, DEV_VENDOR, DEV_PRODUCT)) < 0) {
        wprintw(window_stdout, "Error: ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdic));
        refresh();
        wrefresh(window_stdout);
    }
    ndev = ret; // Discovered devices
    wprintw(window_stdout, "Found %d devices!\n", ndev);
    refresh();
    wrefresh(window_stdout);

    ftdi_cntx.resize(ndev);
    ftdi_error.resize(ndev);
    packet_bytes.resize(ndev);
    for (unsigned int i = 0; i < ndev; i++) {
        packet_bytes[i] = (unsigned char*)calloc(PACKET_SIZE, sizeof(unsigned char));
    }
    for (unsigned int i = 0; i < ndev; i++) {
        ftdi_cntx[i] = (ftdi_context*)calloc(1, sizeof(ftdi_context));
    }

    /* Checking and opening discovered devices */
    i = 0;
    for (curdev = devlist; curdev != NULL; ) {
        wprintw(window_stdout, "Checking dev%d: ", i);
        refresh();
        wrefresh(window_stdout);
        if ((ret = ftdi_usb_get_strings(ftdic, curdev->dev, manufacturer, 128, description, 128, NULL, 0)) < 0) {
            wprintw(window_stdout, "Error: ftdi_usb_get_strings failed: %d (%s)... did you run as sudo?\n", ret, ftdi_get_error_string(ftdic));
            refresh();
            wrefresh(window_stdout);
            goto Error_FTDI;
        }
        wprintw(window_stdout, "%s by %s\n", description, manufacturer);

        if (strncmp(description, DEV_DESCRIPTION, DEV_DESCRIPTION_SIZE) == 0) {

            wprintw(window_stdout, "Initializing ftdi context for dev%d... ", i);
            refresh();
            wrefresh(window_stdout);

            if ((ftdi_cntx[i] = ftdi_new()) == 0) {
                wprintw(window_stdout, "Error: ftdi_new failed for dev%d\n", i);
                refresh();
                wrefresh(window_stdout);
            }
            else wprint(window_stdout, " Success!\n");

            wprintw(window_stdout, "Opening dev%d... ", i);
            refresh();
            wrefresh(window_stdout);

            ftdi_error[i] = ftdi_usb_open_dev(ftdi_cntx[i], curdev->dev);
            if (ftdi_error[i] < 0 && ftdi_error[i] != -5) {
                wprintw(window_stdout, "Error: unable to open dev%d (%s)... did you run as sudo?\n", ret, ftdi_get_error_string(ftdi_cntx[i]));
                refresh();
                wrefresh(window_stdout);
                // std::stringstream err;
                // err << "Error: unable to open ftdi device: " << ftdi_error[i] << " | " << ftdi_get_error_string(ftdi_cntx[i]) << std::endl;
                // wprint(window_stdout, err.str().c_str());
                goto Error_FTDI;
            }
            else wprint(window_stdout, " Success!\n");

            i++;
        }
        else {
            wprintw(window_stdout, "Ignoring device\n", i);
            refresh();
            wrefresh(window_stdout);
        }

        curdev = curdev->next;
    }

    ndev = i;

    if (ndev == 0){
        wprintw(window_stdout, "Exiting because we did not find any %s devices... did you run as sudo?\n", DEV_DESCRIPTION);
        refresh();
        wrefresh(window_stdout);
        sleep(5);
        endwin();
        return EXIT_FAILURE;
    }
    else {
        wprintw(window_stdout, "%d %s devices were found and are ready to be configured\n", ndev, DEV_DESCRIPTION);
        refresh();
        wrefresh(window_stdout);
    }

    /* USB device configuration */
    wprint(window_stdout, "Configuring USB devices...\n");
    for (i = 0; i < ndev; i++) {
        ftdi_set_latency_timer(ftdi_cntx[i], LAT_TIMER);
        ftdi_read_data_set_chunksize(ftdi_cntx[i], READ_CHUNK);
        ftdi_write_data_set_chunksize(ftdi_cntx[i], WRITE_CHUNK);
    }

    /* Start timing */
    // to-do: implement WIN32 timer
    wprint(window_stdout, "Starting clock and log...\n");
    startTime = time(NULL);
    timeval timeStamp;
    gettimeofday(&timeStamp, NULL);
    firstSampleTime = (double)timeStamp.tv_sec + 1e-6*(double)timeStamp.tv_usec;

    /* Output file */
    wprint(window_stdout, "Generating output files... ");
    mkdir("readout/", 0777);
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buf, sizeof(time_buf), "%Y_%m_%d_%H_%M_%S", timeinfo);
    sprintf(data_storage_path, data_storage_folder, time_buf);
    mkdir(data_storage_path, 0777);
    wprintw(window_stdout, "Writing into folder %s ", data_storage_path);
    refresh();
    wrefresh(window_stdout);
    memcpy(file_path, data_storage_path, sizeof(data_storage_path));
    memcpy(file_path2, data_storage_path, sizeof(data_storage_path));
    strcat(file_path, "raw_data.txt");
    strcat(file_path2, "temp_data.txt");
    v_output = fopen(file_path, "wb");
    t_output = fopen(file_path2, "wb");
    assert(v_output != NULL);
    fprintf(v_output, "# thermusb raw voltage data file\n");
    fprintf(v_output, "# Multichannel FPGA Temperature Readout via USB\n");
    fprintf(v_output, "# Logging started %s", ctime(&startTime));
    fprintf(v_output, "# User message: %s\n", usr_var->comment);
    fprintf(v_output, "# Configuration: Thermistor=%s, R2=%.1f, R3=%.1f, R4=%.1f, vs=%.2f, gain=%.3f\n",\
        usr_var->thermistor_name, usr_var->R2, usr_var->R3, usr_var->R4, usr_var->vs, usr_var->g);
    fflush(v_output);
    assert(t_output != NULL);
    fprintf(t_output, "# thermusb calibrated temperature data file\n");
    fprintf(t_output, "# Multichannel FPGA Temperature Readout via USB\n");
    fprintf(t_output, "# Logging started %s", ctime(&startTime));
    fprintf(t_output, "# User message: %s\n", usr_var->comment);
    fprintf(t_output, "# Configuration: Thermistor=%s, R2=%.1f, R3=%.1f, R4=%.1f, vs=%.2f, gain=%.3f\n",\
        usr_var->thermistor_name, usr_var->R2, usr_var->R3, usr_var->R4, usr_var->vs, usr_var->g);
    fflush(t_output);
    wprint(window_stdout, "--- Success!\n");

    /* Timer testing and debugging (uncomment to produce infinite loop) */
    // while(1){
    //     gettimeofday(&timeStamp,NULL);
    //     sampleTime = (double)timeStamp.tv_sec + 1e-6*(double)timeStamp.tv_usec;
    //     fprintf(LogFile,"%13.8f \r",sampleTime - firstSampleTime);
    // }

    /* Header line */
    // time dev1ch0 dev1ch2 ... dev2ch7
    fprintf(v_output, "time ");
    wprintw(window_header, "    time  ");
    for (unsigned int i = 0; i < ndev; i++) {
        for (unsigned int j = 0; j < NCHAN; j++) {
            fprintf(v_output, "dev%dch%d ", i+1, j);
            wprintw(window_header, "dev%dch%d ", i+1, j);
        }
    }
    fprintf(v_output, "\n");
    fflush(v_output);
    refresh();
    wrefresh(window_header);

    fprintf(t_output, "time ");
    for (unsigned int i = 0; i < ndev; i++) {
        for (unsigned int j = 0; j < NCHAN; j++) {
            fprintf(t_output, "dev%dch%d ", i+1, j);
        }
    }
    fprintf(t_output, "\n");
    fflush(t_output);
    refresh();
    wrefresh(window_header);

    wprint(window_stdout,"Entering main loop...\n");
    wprint(window_stdout,"Press Shift+O to switch between voltage and temperature\n");
    wprint(window_stdout,"Press Shift+E to end the program\n");
    
    /* Readout loop (infinite, unless the user stops it) */
    while(run) {

        for (i = 0; i < ndev; i++) {
            do {
                rsize = ftdi_read_data(ftdi_cntx[i], packet_bytes[i], PACKET_SIZE);
		        if (rsize < 0) {
		            wprintw(window_readout, "USB read error (ftdi_read_data returns %d)\n", rsize);
		            sleep(1);
		            refresh();
		            wrefresh(window_readout);
        		}
            } while (rsize < PACKET_SIZE);
        }
        
        gettimeofday(&timeStamp, NULL);
        sampleTime = (long double)timeStamp.tv_sec + 1e-6 * (long double)timeStamp.tv_usec;

        wprintw(window_readout, "%8.1Lf  ", sampleTime - firstSampleTime);
        refresh();
        wrefresh(window_readout);

        fprintf(v_output, "%.14Lf ", sampleTime - firstSampleTime);
        fprintf(t_output, "%.14Lf ", sampleTime - firstSampleTime);

        //checkStatus(packetBytes,win);
        //gps_prep(packet_bytes[0], &clock);
        //gps_to_ymdhms(&clock);
        //gps_to_ymdhms(&clock);

        for (i = 0; i < ndev; i ++) {
            for (n = 0; n < NCHAN; n++) {
                int rawData = AccessNonalignedInt(packet_bytes[i] + CHANNEL_DATA_OFFSET + 1 + n*CHANNEL_BLOCK_SIZE);
                
                if (usr_var->output == 0) {
                    wprintw(window_readout, "%7.4f ", returnVoltage(rawData, usr_var));
                }
                else {
                    wprintw(window_readout, "%7.4f ", convertToTemperature(rawData, usr_var));
                }
                
                refresh();
                wrefresh(window_readout); 

                fprintf(t_output,"%.14f ", convertToTemperature(rawData, usr_var));
                fprintf(v_output,"%.14f ", returnVoltage(rawData, usr_var));
                fflush(t_output);
                fflush(v_output);
            }
        }

        wprint(window_readout,"\n");
        fprintf(v_output,"\n");
        fprintf(t_output,"\n");
        nodelay(stdscr, TRUE);

        c = getch();
        switch(c) {
            case 69: run = 0; break; //Shift+E to end program
            // case 68: break; //Shift+D
            // case 76: break; //Shift+L
            // case 72: break; //Shift+H
            // case 77: break; //Shift+M
            case 79: usr_var->output = (usr_var->output+1)%2; break; //Shift+O to switch GUI output
            // case 80: break; //Shift+P
        }

        } // main loop ends here

        /* Clean up */
        endwin();
	    ftdi_free(ftdic);
        // for (unsigned int i = 0; i < ndev; i++) {
        //     ftdi_free(ftdi_cntx[i]);
        // }
        ftdi_list_free(&devlist);

        /* Program termination */
        if (run == 0) {
            printf("Program ended via Shift+E\n");
        }

        printf("\nGraceful program termination. Goodbye !!\n\n\n");
        return 0;



Error_FTDI:
        sleep(5);
        endwin();
        printf("Program ended due to FTDI error\n");
        return EXIT_FAILURE;

}

