#include <stdio.h>
#include <vector>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits>
#include <predict/predict.h>

#include "testcase_reader.h"

#include <iostream>
using namespace std;

int runtest(const char *filename);

int main(int argc, char **argv)
{
	// Check arguments
	if (argc < 2) {
		cout << "Usage: " << argv[0] << " <testfiles>" << endl;
		return -1;
	}

	// Test all provided test files
	int retval = 0;
	for (int i = 1; i < argc; ++i) {
		if (runtest(argv[i])) {
			cout << argv[i] << ": failed" << endl;
			retval = -1;
		} else {
			cout << argv[i] << ": OK" << endl;
		}
	}
	return retval;
}

int runtest(const char *filename)
{
	// Load testcase
	TestCaseReader testcase;
	testcase.loadFromFile(filename);
	if (!(testcase.containsValidData() && (testcase.containsValidQth()))) {
		fprintf(stderr, "Failed to load testfile: %s\n", filename);
		return -1;
	}
	
	// Create observer object
	predict_observer_t *obs = predict_create_observer("test", testcase.latitude()*M_PI/180.0, testcase.longitude()*M_PI/180.0, testcase.altitude());
	if (!obs) {
		fprintf(stderr, "Failed to initialize observer!");
		return -1;
	}

	// Test
	int retval = 0;
	int line = 1;
	for (auto d : testcase.data()) {
		double time = d[0];
		double az = d[1];
		double el = d[2];

		// Compare values within (time - 1, time + 1) (i.e. up time + 1, but not including time + 1)
		// (since we don't know the exact time predict generated its data, only within an error of 1 second)
		const int DIFF = 1;
		struct predict_observation sun_obs_lower;
		struct predict_observation sun_obs_upper;

		// Lower bound
		predict_observe_sun(obs, predict_to_julian(time), &sun_obs_lower);

		// Upper bound
		predict_observe_sun(obs, predict_to_julian(time + DIFF), &sun_obs_upper);

		// Check values
		string failed = "";
		if (!fuzzyCompareWithBoundaries(sun_obs_lower.azimuth*180.0/M_PI, sun_obs_upper.azimuth*180.0/M_PI, az)) {
			failed += "(azimuth)";
		}
		if (!fuzzyCompareWithBoundaries(sun_obs_lower.elevation*180.0/M_PI, sun_obs_upper.elevation*180.0/M_PI, el)) {
			failed += "(elevation)";
		}

		// Failed?
		if (failed != "") {
			
			cout << filename << ": failed at data line " << line << ": " << failed << endl;

			printf("%.8f, %.8f/%.8f/%.8f, %.8f/%.8f/%.8f\n", time,
					sun_obs_lower.azimuth*180.0/M_PI, az, sun_obs_upper.azimuth*180.0/M_PI,
					sun_obs_lower.elevation*180.0/M_PI, el, sun_obs_upper.elevation*180.0/M_PI);

			retval = -1;
		}

		// Increment data line number
		++line;
	}

	return retval;
}