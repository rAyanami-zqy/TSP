#include <stdio.h>
#include <string.h>
#include <search.h>
#include <stdlib.h>
#include "report.h"

char *algname (char *filename) {

  int n = strlen(filename);
  int i;
  char *s;

  for (i=n-1; i>=0 && filename[i]!= '/'; --i);
  s = (char *)malloc(n-i);
  strncpy (s, filename+i+1,n-i);
  s[n-i-3] = '\0';
  return s;
}

int hashlookup(char *s) {

  ENTRY item;
  ENTRY *result;

  item.key = s;
  result = hsearch (item, FIND);
  if (result == NULL) {
    fprintf (stderr, 
	     "Instance name %s does not appear in the HK bounds file\n", s);
    fprintf (stderr, 
	     "   and will be ignored.\n");
    return -1;
  }
  return (int)(result->data);
}

double *loadOptBounds (int nameCount) {

  double *bounds = (double *)malloc(nameCount*sizeof(double));
  int i;
  FILE *optfile;
  char optfilename[FILENAMELENG];

  strcpy(optfilename, DATAFILEPATH);
  strcat(optfilename, "optvals");
  optfile = fopen(optfilename, "r");
  if (optfile == NULL) {
    fprintf (stderr, "Error opening Opt value file\n");
    exit(12);
  }

  for (i=0; i<nameCount; ++i)
    bounds[i] = -1;

  while (1) {
    char name[NAMELENG];
    double bound;
    int count;

    count = fscanf(optfile, "%s %lf", name, &bound);
    if (count < 2) break;
    i = hashlookup(name);
    if (i < 0) continue;
    bounds[i] = bound;
  }
  fclose(optfile);
  return bounds;
}

int loadHKbounds(char ***names, double **bounds) {

  char buf[MAXLINELENGTH];
  int lines = 0;
  int i;
  int count;
  ENTRY item;
  FILE *hkfile;
  char hkfilename[FILENAMELENG];

  strcpy(hkfilename, DATAFILEPATH);
  strcat(hkfilename, "hkbounds");
  hkfile = fopen(hkfilename, "r");
  if (hkfile == NULL) {
    fprintf (stderr, "Error opening HK bound file\n");
    exit(11);
  }

  while (fgets(buf, MAXLINELENGTH, hkfile) != NULL) lines++;

  *bounds = (double *)malloc(lines*sizeof(double));
  *names = (char **)malloc(lines*sizeof(charptr));

  rewind(hkfile);
  hcreate(lines);

  for (i=0; i<lines; ++i) {
    (*names)[i] = (char *)malloc(NAMELENG);
    count = fscanf (hkfile, "%s %lf", (*names)[i], (*bounds)+i);
    if (count < 2) break;

    item.key = (*names)[i];
    item.data = (void *) i;
    hsearch (item, ENTER);
  }
  fclose(hkfile);
  return i;
}
  
char *readLine(FILE *fp) {

  static char buf[MAXLINELENGTH];
  size_t leng;

  if (fgets (buf, MAXLINELENGTH, fp) == NULL) return NULL;
  leng = strlen(buf);
  if (leng==0) return NULL;
  if (leng==1) return readLine(fp);
  if (buf[leng-1] == '\n') buf[leng-1] = 0;
  return buf;
}

void parseMachineName(char *name, char *line) {

  int i;
  int j=0;

  for (i=0; line[i] != 0 && line[i] != '['; ++i);
  if (line[i] == 0) {name[j]=0; return;}

  for (i++; line[i] != 0 && line[i] == ' '; ++i);
  if (line[i] == 0) {name[j]=0; return;}

  for (; line[i] != 0 && line[i] != ' ' && line[i] != ']'; ++i) 
    name[j++]=line[i];
  name[j]=0;
  return;
}

void loadAlg (FILE *datafile, int instanceCount, double **algresult, 
	      double **algtime, char ***algmem) {

  double *result = (double *) malloc (instanceCount*sizeof(double));
  double *time   = (double *) malloc (instanceCount*sizeof(double));
  char **mem    = (char **) malloc (instanceCount*sizeof(charptr));

  char instance[NAMELENG];
  char *line;
  int i;
  
  for (i=0; i<instanceCount; ++i) {
    result[i] = time[i] = -1;
    mem[i] = NULL;
  }

  while ((line=readLine(datafile)) != NULL) {
    double a, b;
    char *c = (char *) malloc (NAMELENG);
    int count = sscanf (line, "%s %lf %lf %s", instance, &a, &b, c);
    i= hashlookup(instance);
    if (i < 0) continue;
    if (count < 3) break;
    result[i] = a;
    time[i] = b;
    mem[i] = c;
    if (count == 3) {mem[i] = NULL; free(c);}
  }

  *algresult = result;
  *algtime   = time;
  *algmem    = mem;
}
