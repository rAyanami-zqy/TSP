#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "report.h"

int main (int argc, char **argv) {

  char **instanceNames;
  double *hkbounds;
  int instanceCount;

  FILE *datafile;
  char *machineLine;
  char machineName1[MAXLINELENGTH], machineName2[MAXLINELENGTH];
  ratiovec ratios1, ratios2;

  char *alg1name, *alg2name;
  double *alg1result, *alg2result;
  double *alg1time, *alg2time;
  char **alg1mem, **alg2mem;

  double time1, time2;

  int i;

  if (argc != 4) {
    fprintf (stderr, "Usage: algcomp2 file1 file2 showMatrix\n");
    exit(1);
  }

  instanceCount = loadHKbounds(&instanceNames, &hkbounds);

  {
    alg1name = algname(argv[1]);
    datafile = fopen (argv[1], "r");
    if (datafile == NULL) {
      fprintf (stderr, "Error opening data file\n");
      exit(11);
    }
    
    readLine(datafile);
    machineLine = readLine(datafile);
    parseMachineName (machineName1, machineLine);
    readLine(datafile);
    readLine(datafile);
    ratios1 = getNormalizationRatios(machineName1, STDMACHINE);
    
    loadAlg (datafile, instanceCount, &alg1result, &alg1time, &alg1mem);
    fclose (datafile);
  }

  {
    alg2name = algname(argv[2]);
    datafile = fopen (argv[2], "r");
    if (datafile == NULL) {
      fprintf (stderr, "Error opening data file\n");
      exit(11);
    }
    
    readLine(datafile);
    machineLine = readLine(datafile);
    parseMachineName (machineName2, machineLine);
    readLine(datafile);
    readLine(datafile);
    ratios2 = getNormalizationRatios(machineName2, STDMACHINE);
    
    loadAlg (datafile, instanceCount, &alg2result, &alg2time, &alg2mem);
    fclose (datafile);
  }

  printf ("(%s)/(%s) %s %s\n", alg1name, alg2name, alg1name, alg2name);

  for (i=0; i<instanceCount; ++i) {
    if ((isMatrixInstance(instanceNames[i])!=0) 
	&& (strcmp(argv[3],"1")!=0))
      continue;
    if (alg1time[i]>=0.0 && alg2time[i]>=0.0) {
      printf ("%-10s %8d", instanceNames[i], 
	      getInstanceSize(instanceNames[i]));
      if (strcmp(alg1name,"read")==0 || strcmp(alg2name,"read")==0)
	printf (" %6s", "--");
      else
	printf (" %6.2f", 100*(alg1result[i]/alg2result[i] - 1));
      time1 = normalize (instanceNames[i], alg1time[i], ratios1);
      time2 = normalize (instanceNames[i], alg2time[i], ratios2);

      if (time2 > 0) 
	printf (" %10.3f", time1/time2);
      else
	printf (" %10s", "infinity");
      printf ("\n");
    }
    else if (alg1time[i]>=0.0)
      printf ("%-30s %s only\n", instanceNames[i], alg1name);
    else if (alg2time[i]>=0.0)
      printf ("%-30s %s only\n", instanceNames[i], alg2name);
    else
      printf ("%-30s Neither\n", instanceNames[i]);
  }
  return 0;
}
