// main.h
#ifndef MAIN_H
#define MAIN_H

#define NCHAN 8             // Number of channels per device
#define WRITE_CHUNK 180     // Data write chunk size
#define READ_CHUNK 180      // Data read chunk size
#define LAT_TIMER 25        // Latency timer
#define PACKET_SIZE 90      // Data packet size

#define JAN61980 44244
#define JAN11901 15385
#define SEC_PER_DAY 86400.0

// offsets for packet markers
const unsigned int STARTMARKER = 0; // 0xF0
const unsigned int IDXSTATUS0 = 1; // 0x00
const unsigned int STATUS0 = 2;
const unsigned int IDXSTATUS1 = 6; // 0x01
const unsigned int STATUS1 = 7;
const unsigned int CHANNEL_DATA_OFFSET = 11;
const unsigned int CHANNEL_BLOCK_SIZE = 5; // 1 byte for channel number, 4 bytes data
const unsigned int ENDMARKER1 = 87; // 0xAE
const unsigned int ENDMARKER2 = 88; // 0x80
const unsigned int ENDMARKER3 = 89; // 0xFF

// ADC status codes
#define ADC_EOC_ERROR  0x20;
#define ADC_DMY_BIT_ERROR  0x10;
#define ADC_OVERFLOW  0x03;
#define ADC_UNDERFLOW  0x00;
#define ADC_IN_RANGE_NEGATIVE  0x01;
#define ADC_IN_RANGE_POSITIVE  0x02;

struct CLOCK {
	long r_now, f_now;     // number of week of rising and falling edges
	double r_sow, f_sow;   // seconds of week of rising and falling edges
	long r_year, f_year;   // year for rising and falling edges
	long r_month, f_month; // day for rising and falling edges
	long r_day, f_day;     // day for rising and falling edges
	long r_hour, f_hour;   // hours for rising and falling edges
	long r_min, f_min;     // minutes for rising and falling edges
	double r_sec, f_sec;   // seconds for rising and falling edges
};

struct str_def_usr_var {
	char comment[500];              // User comment
	int output;                     // 0 = voltage, 1 = temperature (GUI output)
	double R2;                      // Wheatstone bridge resistors
	double R3;
	double R4;
	double vs;                      // Bridge supply voltage
	double g;                       // Pre-amp gain
	unsigned int thermistor_choice; // 0 = PT10000; 1 = SEMI833ET
	char thermistor_name[20];
	bool debug_mode = false;
};

#endif // MAIN_H