// aux_functions.cpp
#include <ncurses.h>
#include <math.h>
#include "main.h"
#include "aux_functions.h"

#ifdef _WIN32
#else
#include <sys/time.h>
#endif


void gps_to_ymdhms(CLOCK* clock) {
	/*----------------------------------------------------------* 
	 * Original function by Benjamin W. Remondi, November 1999  *
	 *----------------------------------------------------------*/
	static long month_day[2][13] = {
	    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
	    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
	};

	long leap, guess, more;
	long yday, mjd, days_fr_jan1_1901;
	long delta_yrs, num_four_yrs, years_so_far, days_left;
	double fmjd;

	mjd = clock->r_now * 7 + clock->r_sow / SEC_PER_DAY + JAN61980;
	fmjd = fmod (clock->r_sow, SEC_PER_DAY) / SEC_PER_DAY;

	days_fr_jan1_1901 = mjd - JAN11901;
	num_four_yrs = days_fr_jan1_1901 / 1461;
	years_so_far = 1901 + 4 * num_four_yrs;
	days_left = days_fr_jan1_1901 - 1461 * num_four_yrs;
	delta_yrs = days_left / 365 - days_left / 1460;

	clock->r_year = years_so_far + delta_yrs;
	yday = days_left - 365 * delta_yrs + 1;
	clock->r_hour = fmjd * 24.0;
	clock->r_min = fmjd * 1440.0 - clock->r_hour * 60.0;
	clock->r_sec = fmjd * 86400.0 - clock->r_hour * 3600.0 - clock->r_min * 60.0;

	leap = (clock->r_year % 4 == 0);
	guess = yday * 0.032;
	more = ((yday - month_day[leap][guess + 1]) > 0);
	clock->r_month = guess + more + 1;
	clock->r_day = yday - month_day[leap][guess + more];
}


void gps_prep(unsigned char *packet, CLOCK* clock) {
  clock->r_now=256*packet[62]+packet[61];
  clock->f_now=256*packet[64]+packet[63];

  clock->r_sow=(pow(256.0, 3.0)*packet[68]+pow(256.0, 2.0)*packet[67]+256*packet[66]+packet[65])/pow(10.0, 3.0)+
  (pow(256.0, 3.0)*packet[72]+pow(256.0, 2.0)*packet[71]+256*packet[70]+packet[69])/pow(10.0, 9.0);

  clock->f_sow=(pow(256.0, 3.0)*packet[76]+pow(256.0, 2.0)*packet[75]+256*packet[74]+packet[73])/pow(10.0, 3.0)+
  (pow(256.0, 3.0)*packet[80]+pow(256.0, 2.0)*packet[79]+256*packet[78]+packet[77])/pow(10.0, 9.0);
}


void decodeStatus(const unsigned char status, const unsigned int channel, WINDOW *win) {
	switch (status) {
	    case 0x02 : //ADC_IN_RANGE_POSITIVE (normal status)
		    break;
	    case 0x01 : //ADC_IN_RANGE_NEGATIVE (normal status)
		    break;
	    case 0x20 : //ADC_EOC_ERROR
			//printf("C%d: EOC ",channel);
			wprintw(win,"C%d: EOC ",channel);
			refresh();
			wrefresh(win);
			break;
	    case 0x10 : //ADC_DMY_BIT_ERROR :
			//printf("C%d: DMY ",channel);
		    wprintw(win,"C%d: DMY ",channel);
		    refresh();
		    wrefresh(win);
		    break;
	    case 0x03 : //ADC_OVERFLOW :
			//printf("C%d: OVF ",channel);
			wprintw(win,"C%d: OVF ",channel);
			refresh();
			wrefresh(win);
			break;
		case 0x00 : //ADC_UNDERFLOW :
			//printf("C%d: UDF ",channel);
			wprintw(win,"C%d: UDF ",channel);
			refresh();
			wrefresh(win);
			break;
		default :
			// printf("C%d: %02X ",channel,status);
			wprintw(win,"C%d: %02X ",channel,status);
			refresh();
			wrefresh(win);
			break;
	}
}

void checkStatus(unsigned char *packetBytes, WINDOW *win) {

	unsigned int offset;
	unsigned int m;

	// WARNING This section of code is highly dependent on the packet format and will be
	// easily broken by changes to the packet format.
	for(m = 0; m < NCHAN; m++) {
		if (m<=3) {
			offset = STATUS0 + m;
		} 
		else {
			offset = STATUS1 + m - 4;
		}
	// first measurement
	decodeStatus(packetBytes[offset] & 0x0F, m, win);
	// r_second measurement
	decodeStatus(packetBytes[offset] >> 4, m, win);
	}
}


double u2r(const double u, str_def_usr_var *usr_var) {
	// Convert Wheatstone bridge voltage to thermistor resistance

	// return usr_var->R2 * (1 / (u / usr_var->vs + usr_var->R4 / (usr_var->R3 + usr_var->R4)) - 1);
	return (usr_var->R3 * usr_var->R4 * u + usr_var->R2 * usr_var->R3 * (usr_var->vs + u)) / (usr_var->R4 * usr_var->vs - (usr_var->R2 + usr_var->R4) * u);
}


double r2t(const double r, str_def_usr_var *usr_var) {
	// Convert thermistor resistance to temperature in degrees Celsius (unit? Ohm? kOhm?)

	if (usr_var->thermistor_choice == 0) {
		static const double R0 = 10e3;     // PT10000 resistance at 0 celsius
		static const double A = 3.9083e-3; // Standard constants for PTCs (from DIN EN 60751)
		static const double B = -5.775e-7;
		return (-R0 * A + sqrt((R0 * A) * (R0 * A) - 4.0 * R0 * B * (R0 - r))) / (2.0 * R0 * B);
	}
	else if (usr_var->thermistor_choice == 1) {
		// third order polinomial calibration
		static const double a = -3.423e-7;
		static const double b =  1.495e-4;
		static const double c = -0.05063;
		static const double d = 12.5;

		double p = c/a - pow(b, 2)/(3*pow(a, 2));
		double q = ( d-log(r) )/a - b*c/(3*pow(a, 2)) + 2*pow(b, 3)/(27*pow(a, 3));

		double delta = pow(q, 2)/4 + pow(p, 3)/27;

		double r__plus = -q/2 + sqrt(delta);
		double r_minus = -q/2 - sqrt(delta);

		return -b/(3*a) + cbrt(r__plus) + cbrt(r_minus);
	}
	else exit(0);
}


double convertToTemperature(const int rawData, str_def_usr_var *usr_var) {
	//    2.5 = 0.5*Vref (Vref=reference voltage for ADC);
	//    2^28 = 268435456;

	// FPGA adds to measurements but does not divide by two
	// thus must divide here by two in order to properly implement averaging

	double voltage = (double) rawData / 2.0 / ((double)268435456) * 2.5 / usr_var->g;

	// return voltage;
	return r2t(u2r(voltage, usr_var), usr_var);
}


double returnVoltage(const int rawData, str_def_usr_var *usr_var) {

	double voltage = (double) rawData / 2.0 / ((double)268435456) * 2.5 / usr_var->g;
	
	return voltage;
}


// Returns a 4 byte integer that has proper byte alignment from a pointer to
// a 4 byte integer that does not have the proper byte alignment
//
// USE THIS FUNCTION WITH CARE (esp. w.r.t. endian issues)
int AccessNonalignedInt(unsigned char *address) {
	return (address[0] << 24) | (address[1] << 16) | (address[2] << 8) | address[3];
}
