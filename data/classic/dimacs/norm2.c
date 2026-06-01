#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "report.h"

int main (int argc, char **argv) {

  char **instanceNames;
  double *hkbounds;
  double *optbounds;
  int instanceCount;

  FILE *datafile;
  char *machineLine;
  char machineName[MAXLINELENGTH];
  ratiovec ratios;

  int rate;

  double *alg1result;
  double *alg1time;
  char **alg1mem;
  char *normMachine;

  if (argc != 4) {
    fprintf (stderr, "Usage: norm2 filename growthRate showMatrix\n");
    exit(1);
  }

  normMachine = STDMACHINE;

  if (strcmp(argv[2], "1") == 0) rate=0;
  else if (strcmp(argv[2], "N") == 0) rate=1;
  else if (strcmp(argv[2], "NlogN") == 0) rate=2;
  else if (strcmp(argv[2], "Nlog^2N") == 0) rate=3;
  else if (strcmp(argv[2], "N^1.25") == 0) rate=4;
  else if (strcmp(argv[2], "N^1.5") == 0) rate=5;
  else if (strcmp(argv[2], "N^2") == 0) rate=6;
  else if (strcmp(argv[2], "N^2.5") == 0) rate=7;
  else if (strcmp(argv[2], "N^3") == 0) rate=8;
  else {
    fprintf (stderr, "Growth rate not recognized\n");
    exit(2);
  }

  instanceCount = loadHKbounds(&instanceNames, &hkbounds);
  optbounds = loadOptBounds (instanceCount);

  datafile = fopen (argv[1], "r");
  if (datafile == NULL) {
    fprintf (stderr, "Error opening data file\n");
    exit(11);
  }

  readLine(datafile);
  machineLine = readLine(datafile);
  parseMachineName (machineName, machineLine);
  readLine(datafile);
  readLine(datafile);
  ratios = getNormalizationRatios(machineName, normMachine);

  loadAlg (datafile, instanceCount, &alg1result, &alg1time, &alg1mem);

  {
    int i;
    double time;
    double cities, logcities;

    for (i=0; i<instanceCount; ++i) {
      if (alg1result[i] >= 0) {
	if ((isMatrixInstance(instanceNames[i])!=0) 
	    && (strcmp(argv[3],"1")!=0))
	  continue;
	printf ("%s ", instanceNames[i]);
	cities = getInstanceSize(instanceNames[i]);
	logcities = log(cities);
	printf ("%.0f ", cities);
	printf ("%.2f ", alg1result[i]/hkbounds[i]*100 - 100);
	time = normalize (instanceNames[i],alg1time[i], ratios);
	time *= 1000000;
	switch (rate) {
	case 0:
	  break;
	case 1:
	  time /= cities;
	  break;
	case 2:
	  time /= (cities * logcities);
	  break;
	case 3:
	  time /= (cities * logcities * logcities);
	  break;
	case 4:
	  time /= (cities * sqrt(sqrt(cities)));
	  break;
	case 5:
	  time /= (cities * sqrt(cities));
	  break;
	case 6:
	  time /= (cities * cities);
	  break;
	case 7:
	  time /= (cities * cities * sqrt(cities));
	  break;
	case 8:
	  time /= (cities * cities * cities);
	  break;
	default:
	  fprintf (stderr, "Strange case in norm2\n");
	  exit(3);

	}
	printf ("%.2f ", time);
	printf("\n");
      }
    }
  }
  return 0;
}
