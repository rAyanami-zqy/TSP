#include <stdio.h>
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

  double *alg1result;
  double *alg1time;
  char **alg1mem;
  char *normMachine;

  if (argc != 2 && argc != 3) {
    fprintf (stderr, "Usage: norm1 datafile\n");
    fprintf (stderr, "   or\n");
    fprintf (stderr, "       norm1 datafile normMachine\n");
    exit(1);
  }

  if (argc == 2)
    normMachine = STDMACHINE;
  else
    normMachine = argv[2];
  
  instanceCount = loadHKbounds(&instanceNames, &hkbounds);
  optbounds = loadOptBounds (instanceCount);

  datafile = fopen (argv[1], "r");
  if (datafile == NULL) {
    fprintf (stderr, "Error opening data file\n");
    exit(11);
  }

  printf ("%s\n", readLine(datafile));
  machineLine = readLine(datafile);
  parseMachineName (machineName, machineLine);
  printf ("%s\n", machineLine);
  printf ("%s\n", readLine(datafile));
  printf ("%s\n", readLine(datafile));

  if (strcmp(normMachine, STDMACHINE) != 0)
    printf ("Normalized times are for %s\n", normMachine);
  printf ("\n");

  ratios = getNormalizationRatios(machineName, normMachine);

  loadAlg (datafile, instanceCount, &alg1result, &alg1time, &alg1mem);

  {
    int i;

    {
      printf("%10s ","INSTANCE");
      printf("%12s ","Tour length");
      printf("%6s ", "% Opt");
      printf("%6s ", "% HK");
      printf("%10s ","CPU Time");
      printf("%10s ","Norm Time");
      printf("%6s","Memory");
      printf ("\n");
    }

    for (i=0; i<instanceCount; ++i) {
      printf ("%10s ", instanceNames[i]);
      if (alg1result[i] >= 0) {
	printf ("%12.0f ", alg1result[i]);
	if (optbounds[i] >= 0) printf ("%6.2f ", 
				       alg1result[i]/optbounds[i]*100 - 100);
	else
	  printf ("%6s ", "?");
	printf ("%6.2f ", alg1result[i]/hkbounds[i]*100 - 100);      
	printf ("%10.2f ", alg1time[i]);
	printf ("%10.2f ", normalize (instanceNames[i],alg1time[i], ratios));
	if (alg1mem[i] != NULL)
	  printf ("%6s", alg1mem[i]);
	else
	  printf ("%6s", "--");
      }
      else {
	printf("%12s ","--");
	printf("%6s ","--");
	printf("%6s ","--");
	printf("%10s ","--");
	printf("%10s ","--");
	printf("%6s","--");
      }
      printf("\n");
    }
  }
  return 0;
}
