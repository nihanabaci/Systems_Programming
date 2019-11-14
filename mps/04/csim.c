
#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void parseInput(int argc, char **argv);
void parseAdd(unsigned long add);

int s;
int E;
int b;
char *t;
unsigned long Add1[3];
int hit = 0;
int miss = 0;
int evic = 0;
int globC = 0;

typedef struct {
    int valid;
    unsigned long tag;
    int counter;
} line;

typedef struct {
    line *lines;
} set;

typedef struct {
    set *sets;
} cache;


int main(int argc, char **argv)
{
  
    parseInput(argc, argv);
    

    set *se = calloc(((int)pow(2,s)),sizeof(set));

    cache c = {.sets = se};

    int i = 0;
    set *sp = c.sets;
    for (i=0; i<((int)pow(2,s)); i++) {
        line *l = calloc(E,sizeof(line));
        int j = 0;
        for (j=0; j<E; j++) {
            (l+j)->tag = -1;
        }
        (sp + i)->lines = l;
    }

  
    FILE *file = fopen(t, "r"); 
    char fChar, sChar, wChar;
    unsigned long add, blk;

    while ( fscanf(file, "%c %c %lx %c %lx", &fChar, &sChar, &add, &wChar, &blk) == 5) {
      
        parseAdd(add);
      
        line *cl = (c.sets + Add1[1])->lines;

        int i = 0;
      

        if (sChar == 'L' || sChar == 'S') {
            int missed = 0;
            for (i=0; i<E; i++) {
                if ((cl+i)->tag == Add1[2]) {
                    hit += 1;
                    (cl+i)->counter = globC;
                    globC += 1;
                    missed = 0;
                    break;
                } else {
                    missed = 1;
                }
            }
            if (missed) {
                miss += 1;
                int lru = globC + 100000;
                line *l;
                int iLru = 0;
              
                int j = 0;
                for (j=0; j<E; j++) {
                  
                    if (!(cl+j)->valid) {
                        l = (cl+j);
                        l->valid = 1;
                        l->tag = Add1[2];
                        l->counter = globC;
                        globC += 1;
                        iLru = 0;
                        
                        break;
                  
                    } else {
                        iLru =1;
                    }
                }

                if (iLru) {
                    for (j=0; j<E; j++) {
                        if (lru>(cl+j)->counter) {
                            lru = (cl+j)->counter;
                            l = (cl+j);
                        }
                    }
                    if ((l->valid) == 1){
                        evic += 1;
                    }
                    l->valid = 1;
                    l->tag = Add1[2];
                    l->counter = globC;
                    globC += 1;
                  
                }
            }
        } else if (sChar == 'M') {
            int r = 0;
            for (r=0; r<2; r++) {
                int missed = 0;
                for (i=0; i<E; i++) {
                    if ((cl+i)->tag == Add1[2]) {
                        hit += 1;
                        (cl+i)->counter = globC;
                        globC += 1;
                        missed = 0;
                        break;
                    } else {
                        missed = 1;
                    }
                }
                if (missed) {
                    miss += 1;
                    int lru = globC + 100000;
                    line *l;
                    int iLru = 0;
                  
                    int j = 0;
                    for (j=0; j<E; j++) {
                        
                        if (!(cl+j)->valid) {
                            l = (cl+j);
                            l->valid = 1;
                            l->tag = Add1[2];
                            l->counter = globC;
                            globC += 1;
                            iLru = 0;
                          
                            break;
                      
                        } else {
                            iLru = 1;
                        }
                    }
                    if (iLru) {
                        for (j=0; j<E; j++) {
                            if (lru>(cl+j)->counter) {
                                lru = (cl+j)->counter;
                                l = (cl+j);
                            }
                        }
                        if ((l->valid) == 1){
                            evic += 1;
                        }
                        l->valid = 1;
                        l->tag = Add1[2];
                        l->counter =globC;
                        globC += 1;
                      
                    }
                }
            }
        }
    }

    fclose(file);

    printSummary(hit, miss, evic);
    return 0;
}


void parseInput(int argc, char **argv) {
    if (argc < 8) {
        printf("Not all arguments added!");
    } else {
        s = atoi(argv[2]);
        E = atoi(argv[4]);
        b = atoi(argv[6]);
        t = argv[8];
    }
}


void parseAdd(unsigned long add) {
  
    unsigned long mask = ((1 << b)-1);
    unsigned long bBits = add & mask;
    Add1[0] = bBits;
    mask = ((1 << s)-1) << b;
    unsigned long sBits = add & mask;
    Add1[1] = sBits >> b;
    mask = ~((1 << (s+b))-1);
    unsigned long tBits = add & mask;
    Add1[2] = tBits >> (s+b);

  

}

