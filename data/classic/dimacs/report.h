#include <stdio.h>

#define MAXLINELENGTH 256
#define NAMELENG 20
#define FILENAMELENG 256
#define STDMACHINE "alpha500"
#define DATAFILEPATH ""

typedef char *charptr;
typedef double *ratiovec;

extern ratiovec getNormalizationRatios(char *machineName, char *normMachine);
extern double normalize (char *instance, double time, ratiovec v);
extern int getInstanceSize(char *instance);
extern int isMatrixInstance(char *instance);

extern char *algname(char *filename);
extern int hashlookup(char *s);
extern double *loadOptBounds (int nameCount);
extern int loadHKbounds(char ***names, double **bounds);
extern char *readLine(FILE *fp);
extern void parseMachineName(char *name, char *line);
extern void loadAlg (FILE *datafile, int instanceCount, double **algresult, 
		     double **algtime, char ***algmem);
















