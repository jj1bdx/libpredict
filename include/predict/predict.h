#ifndef _PREDICT_H_
#define _PREDICT_H_

#include <time.h>
#include <stdbool.h>

/* The representation of time used by libpredict: The number of days since 31Dec79 00:00:00 UTC. */
typedef double predict_julian_date_t;

/* Convert time_t to julian date in UTC. */
predict_julian_date_t predict_get_julian_date_from_time(time_t time);

/* Convert julian date in UTC back to a time_t. */
time_t predict_get_time_from_julian_date(predict_julian_date_t date);

//Seems to be used as a container for processed tle data from
//the original TLE.
typedef struct	{
	double epoch;
	double xndt2o;
	double xndd6o;
	double bstar;
	double xincl;
	double xnodeo;
	double eo;
	double omegao;
	double xmo;
	double xno;
 	int catnr;
	int elset;
	int revnum;
}tle_t; 

typedef enum {
  EPHEMERIS_SGP4 = 0,
  EPHEMERIS_SDP4 = 1,
  EPHEMERIS_SGP8 = 2,
  EPHEMERIS_SDP8 = 3
} ephemeris_t;

struct orbit {
	char name[128];
	double time;
	double position[3];
	double velocity[3];

	double latitude;
	double longitude;
	double altitude;
	int eclipsed;
	ephemeris_t ephemeris;
	///Original TLE line number one:
	char line1[70];
	///Original TLE line number two:
	char line2[70];
	///Original tle_t used to hold processed tle parameters used in calculations.
	tle_t tle;

	///Satellite number (line 1, field 2)
	long catnum;
	///Element number (line 1, field 13)
	long setnum;
	///International designator (line 1, fields 4, 5, 6)
	char designator[10];
	///Epoch year (last two digits) (line 1, field 7)
	int year;
	///Epoch day (day of year and fractional portion of day, line 1, field 8)
	double refepoch;
	///Inclination (line 2, field 3)
	double incl;
	///Right Ascension of the Ascending Node [Degrees] (line 2, field 4)
	double raan;
	///Eccentricity (decimal point assumed) (line 2, field 5)
	double eccn;
	///Argument of Perigee [Degrees] (line 2, field 6)
	double argper;
	///Mean Anomaly [Degrees] (line 2, field 7)
	double meanan;
	///Mean Motion [Revs per day] (line 2, field 8)
	double meanmo;
	///First Time Derivative of the Mean Motion divided by two (line 1, field 9)
	double drag;
	///Second Time Derivative of Mean Motion divided by six (decimal point assumed, line 1, field 10)
	double nddot6;
	///BSTAR drag term (decimal point assumed, line 1, field 11)
	double bstar;
	///Orbital number (line 2, field 9)
	long orbitnum;

	///Ephemeris data structure pointer
	void *ephemeris_data;

};

typedef struct orbit orbit_t;

orbit_t *orbit_create(const char *tle[]);
void orbit_destroy(orbit_t *orbit);

int orbit_predict(orbit_t *x, predict_julian_date_t time);
bool orbit_is_geostationary(const orbit_t *x);
double orbit_apogee(const orbit_t *x);
double orbit_perigee(const orbit_t *x);
bool orbit_aos_happens(const orbit_t *x, double latitude);

/* return true if orbit has decayed */
bool orbit_decayed(const orbit_t *x);

typedef struct observer {
	///Observatory name
	char name[128];
	///Latitude (WGS84, radians)
	double latitude;
	///Longitude (WGS84, radians)
	double longitude;
	///Altitude (WGS84, meters)
	double altitude;
} observer_t;

struct observation {
	///UTC time                
	double time;                       
	///Azimut angle      
	double azimut; 
	///Elevation angle                               
	double elevation; 
	///Corrected elevation    
	double correctedElevation;         
	///Range (km) 
	double range;                        
	///Range vector                    
	double xRange, yRange, zRange; 
	///Range velocity (km/s) 
	double rangeDot;                    
	///Visible? 
	int visible; 
};

observer_t *observer_create(const char *name, double lat, double lon, double alt);
void observer_destroy(observer_t *obs);

void observer_find_orbit(const observer_t *observer, const orbit_t *orbit, struct observation *obs);
void observer_find_moon(const observer_t *observer, predict_julian_date_t time, struct observation *obs);
void observer_find_sun(const observer_t *observer, predict_julian_date_t time, struct observation *obs);

/* Find next AOS from time start_time (ignore previous AOS of current pass if satellite is in range) */
predict_julian_date_t observer_get_next_aos(const observer_t *observer, orbit_t *orbit, predict_julian_date_t start_time);

/* Find next LOS from time start_time (LOS of an upcoming pass or the current pass if satellite is in range) */
predict_julian_date_t observer_get_next_los(const observer_t *observer, orbit_t *orbit, predict_julian_date_t start_time);

/* Calculate doppler shift of a given downlink frequency with respect to the observer */
double observer_get_doppler_shift(const observer_t *observer, const orbit_t *orbit, double downlink_frequency);

#endif //_PREDICT_H_
