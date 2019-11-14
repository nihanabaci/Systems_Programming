/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void trans32(int M, int N, int A[N][M], int B[M][N]);
void trans64(int M, int N, int A[N][M], int B[M][N]);
void trans61(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
if (M == 32) {
        trans32(M,N,A,B);
    } else if (M == 64 ) {
        trans64(M,N,A,B);
    } else if(M == 61) {
        trans61(M,N,A,B);
    }

}


/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
char trans32_desc[] = "32-32:";
void trans32(int M, int N, int A[N][M], int B[M][N]) {
    
    
    int i, j, k, l;
    int tmp;

    for(i=0; i<M; i+=8) {
        for(j=0; j<N; j+=8) {
            
            for(k=i+7; (k>(i+3)) && k<M; k--) {
                for(l=j+4; (l<=(j+7)) && l<N; l++) {
                    B[l-4][k-4]=A[k][l];
                }
            }

            for(k=i+7; (k>(i+3)) && k<M; k--) {
                for(l=j+3; (l>=j) && l<N; l--) {
                    B[l][k]=A[k][l];
                }
            }

            for(k=i; (k<=(i+3)) && k<M; k++) {
                for(l=j; (l<=(j+3)) && l<N; l++) {
                    B[l+4][k+4]=A[k][l];
                }
            }

            for(k=i; (k<=(i+3)) && k<M; k++) {
                for(l=j+4; (l<=(j+7)) && l<N; l++) {
                    B[l][k]=A[k][l];
                }
            }

            for(k=i; (k<=(i+3)) && k<M; k++) {
                for(l=j; (l<=(j+3)) && l<N; l++) {
                    tmp=B[k+4+(j-i)][l+4+(i-j)];
                    B[k+4+(j-i)][l+4+(i-j)]=B[k+(j-i)][l+(i-j)];
                    B[k+(j-i)][l+(i-j)]=tmp;
                }
            }
        }

    }

}


char trans64_desc[] = "64-64:";
void trans64(int M, int N, int A[N][M], int B[M][N]) {
  
    
    int i , j, k, l, f, g;
    int tmp;

    for(i=0; i<M; i+=8) {
        for(j=0; j<N; j+=8) {
            f=j-i;
            g=i-j;
            
            for(k=i+2; k<(i+4); k++) {
                for(l=j; l<(j+8); l++) {
                    B[k-2+f][l+g]=A[k][l];
                }
            }

            for(k=i; k<(i+2); k++) {
                for(l=j; l<(j+8); l++) {
                    B[k+2+f][l+g]=A[k][l];
                }
            }

            for(k=i; k<(i+2); k++) {
                for(l=j; l<(j+8); l++) {
                    tmp=B[k+f][l+g];
                    B[k+f][l+g]=B[k+2+f][l+g];
                    B[k+2+f][l+g]=tmp;
                }
            }
            
            for(k=i+f; k<(i+4+f); k++) {
                for(l=j+g; l<(j+4+g); l++) {
                    if((l-(j+g))<(k-(i+f))) {
                        tmp=B[k][l];
                        B[k][l]=B[l+f][k+g];
                        B[l+f][k+g]=tmp;
                    }
                }
            }
            
            for(k=i+f; k<(i+4+f); k++) {
                for(l=j+4+g; l<(j+8+g); l++) {
                    if((l-(j+4+g))<(k-(i+f))) {
                        tmp=B[k][l];
                        B[k][l]=B[l+f-4][k+g+4];
                        B[l+f-4][k+g+4]=tmp;
                    }
                }
            }
        
            for(k=i; k<(i+2); k++) {
                for(l=j+4; l<(j+8); l++) {
                    tmp=B[k+f][l+g];
                    B[k+f][l+g]=B[k+2+f][l+g];
                    B[k+2+f][l+g]=tmp;
                }
            }

            for(k=i+4; k<(i+6); k++) {
                for(l=j; l<(j+8); l++) {
                    B[k+2+f][l+g]=A[k][l];
                }
            }
            
            for(k=i+6; k<(i+8); k++) {
                for(l=j; l<(j+8); l++) {
                    B[k-2+f][l+g]=A[k][l];
                }
            }
            
            for(k=i+4; k<(i+6); k++) {
                for(l=j; l<(j+8); l++) {
                    tmp=B[k+f][l+g];
                    B[k+f][l+g]=B[k+2+f][l+g];
                    B[k+2+f][l+g]=tmp;
                }
            }
            
            for(k=i+4+f; k<(i+8+f); k++) {
                for(l=j+g; l<(j+4+g); l++) {
                    if((l-(j+g))<(k-(i+4+f)))
                    {
                        tmp=B[k][l];
                        B[k][l]=B[l+f+4][k+g-4];
                        B[l+f+4][k+g-4]=tmp;
                    }
                }
            }
            
            for(k=i+4+f; k<(i+8+f); k++) {
                for(l=j+4+g; l<(j+8+g); l++) {
                    if((l-(j+4+g))<(k-(i+4+f)))
                    {
                        tmp=B[k][l];
                        B[k][l]=B[l+f][k+g];
                        B[l+f][k+g]=tmp;
                    }
                }
            }
            
            for(k=i+4; k<(i+6); k++) {
                for(l=j+0; l<(j+4); l++) {
                    tmp=B[k+f][l+g];
                    B[k+f][l+g]=B[k+2+f][l+g];
                    B[k+2+f][l+g]=tmp;
                }
            }

            for(k=i+4; k<(i+6); k++) {
                for(l=j+0; l<(j+4); l++) {
                    tmp=B[k+f][l+g];
                    B[k+f][l+g]=B[k-2+f][l+4+g];
                    B[k-2+f][l+4+g]=tmp;
                }
            }

            for(k=i+6; k<(i+8); k++) {
                for(l=j+0; l<(j+4); l++) {
                    tmp=B[k+f][l+g];
                    B[k+f][l+g]=B[k-6+f][l+4+g];
                    B[k-6+f][l+4+g]=tmp;
                }
            }
        }
    }
}


char trans61_desc[] = "61-67:";
void trans61(int M, int N, int A[N][M], int B[M][N]) {
  

    int i,j,k,l;
    int tmp;

    for(i=0; i<M; i+=18) {
        for(j=0; j<N; j+=18) {
            for(k=i; (k<(i+18)) && (k<M); k++) {
                for(l=j; (l<(j+18)) && l<N; l++) {
                    if((k)==(l)) {
                        tmp=A[k][k];
                    }
                    else {
                        B[k][l]=A[l][k];
                    }
                }
                if(i==j) {
                    B[k][k]=tmp;
                }

            }

        }
    }
}






void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
   

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

