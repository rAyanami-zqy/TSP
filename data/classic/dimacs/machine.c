#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "report.h"

int sizes[] = { 1000, 3162, 10000, 31623, 100000, 316228, 1000000, 3162278,
	       10000000};
const int sizecount = sizeof(sizes)/sizeof(int);

typedef double *timevec;

static timevec getMachineTimes (char *c) {
  timevec time = (timevec) malloc (sizecount * sizeof(double));
  FILE *infile;
  int i, count, size, iterations, secs, j;
  char filename[FILENAMELENG];

  for (i=0; i<sizecount; ++i)
    time[i] = -1;

  strcpy (filename, DATAFILEPATH);
  strcat (filename, "s.");
  strcat (filename, c);
  infile = fopen (filename, "r");

  if (infile == NULL) {
    fprintf (stderr, "Error opening machine description file %s\n", filename);
    exit(1);
  }

  for (i=0; i<sizecount; ++i) {
    count = fscanf (infile, "%d %d %d", &size, &iterations, &secs);
    if (count < 3) break;

    for (j=0; j<sizecount; ++j)
      if (size == sizes[j]) {
	time[j] = (double)secs/iterations;
	break;
      }
    if (j >= sizecount) {
      fprintf (stderr, "Unexpected instance size %d found in file %s.\n",
	       size, filename);
      exit(2);
    }
  }
  fclose (infile);
  return time;
}

static ratiovec normalize2(timevec w, char *normmachine) {

  timevec v = getMachineTimes (normmachine);
  int i, j, k;

  for (i=0; i<sizecount; ++i) {
    if (w[i] >= 0) v[i] = w[i] / v[i];
    else v[i] = -1;

    v[i] = 1 / v[i];    /* xxx: temp */
  }

  for (i=0; i<sizecount; ++i) {
    if (v[i] == -1) {
      for (j=i-1; j>=0 && v[j]==-1; --j);
      for (k=i+1; k<sizecount && v[k]==-1; ++k);
      if (j<0) {
	if (k==sizecount) {
	  fprintf (stderr, "weirdness in normalize2()\n");
	  exit(3);
	}
	v[i]=v[k];
      }
      else {
	if (k==sizecount)
	  v[i]=v[j];
	else
	  v[i] = (v[k]-v[j])/(sizes[k]-sizes[j])*(sizes[i]-sizes[j]) +v[j];
      }
    }
  }
  return v;
}

ratiovec getNormalizationRatios (char *machineName, char * normMachine) {
  return normalize2(getMachineTimes(machineName), normMachine);
}

int isMatrixInstance(char *instanceName) {
  if (instanceName[0] == 'M') return 1;
  else if (strstr(instanceName, "si") != NULL) return 1;
  else return 0;
}

int getInstanceSize(char *instanceName) {
  int cities;
  int i;

  if (strstr(instanceName, "316k")!=NULL) cities = sizes[5];
  else if (strstr(instanceName, "100k")!=NULL) cities = sizes[4];
  else if (strstr(instanceName, "31k")!=NULL) cities = sizes[3];
  else if (strstr(instanceName, "10k")!=NULL) cities = sizes[2];
  else if (strstr(instanceName, "10M")!=NULL) cities = sizes[8];
  else if (strstr(instanceName, "1k")!=NULL) cities = sizes[0];
  else if (strstr(instanceName, "3k")!=NULL) cities = sizes[1];
  else if (strstr(instanceName, "1M")!=NULL) cities = sizes[6];
  else if (strstr(instanceName, "3M")!=NULL) cities = sizes[7];
  else {
    for (i=(int)strlen(instanceName)-1; i>=0 && isdigit((int)instanceName[i]);
	 --i);
    cities = atoi(instanceName+i+1);
  }

  return cities;
}
  
double normalize (char *instanceName, double time, ratiovec v) {

  int cities = getInstanceSize(instanceName);
  int i;

  for (i=0; i<sizecount; ++i) {
    if (cities == sizes[i]) return time * v[i];
    if (cities < sizes[i]) {
      if (i == 0) return time * v[0];
      else {
	double ratio = (v[i]-v[i-1])/(sizes[i]-sizes[i-1])*(cities-sizes[i-1])
	  + v[i-1];
	/*	printf ("\nRatio for %d is %7.3lf\n", sizes[i-1], v[i-1]);
	printf ("Ratio for %d is %7.3lf\n", sizes[i], v[i]);
	printf ("Ratio for %d is %7.3lf\n", cities, ratio); */
	return time * ratio;
      }
    }
  }    
  return time * v[sizecount-1];
}

