/*
 *  Compilation et exécution:
 *  1) gcc -o sequentiel sequentiel.c 
 *  2) ./sequentiel 10 4   <Argument 1 : Taille de la matrice, Argument 2 : seed aléatoire>.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAXN 20000  /* Valeur max de N */
int N;
/* Matrices et vecteurs */
volatile float A[MAXN][MAXN], B[MAXN], X[MAXN];


void parameters(int argc, char **argv);
void initializeInputs();
void printInputs();
void printX();
void backSubstitution();
void gaussSeq();



int main(int argc, char **argv) {
    /*Variables pour prélever les temps */
    struct timeval etStart, etStop;  /* Temps d'exécution sera calculé avec gettimeofday() */
    struct timezone dummyTz;
    unsigned long long startTime, endTime;

    /* Paramètres du programme */
    parameters(argc, argv);
    /* Initialiser des matrices A et B */
    initializeInputs();
    /* Afficher les matrices générées */
    printInputs();
    /**Avant l'exécution du code de résolution Gauss-Jordan */
    gettimeofday(&etStart, &dummyTz);
    /* Exécution*/
    gaussSeq();
    /**Temps d'exécution*/
    gettimeofday(&etStop, &dummyTz);
    /* Afficher la solution X */
    printX();
    startTime = (unsigned long long)etStart.tv_sec * 1000000 + etStart.tv_usec;
    endTime = (unsigned long long)etStop.tv_sec * 1000000 + etStop.tv_usec;
    /* Display timing results */
    printf("\nTemps d'exécution Séquentiel = %g ms.\n",(float)(endTime - startTime)/(float)1000);

    exit(0);

}


void parameters(int argc, char **argv) {
    int seed = 0;
  if(argc == 3){
        seed = atoi(argv[2]);
        srand(seed);

        N = atoi(argv[1]);
        if (N < 1 || N > MAXN) {
            printf("N = %i est plus grand que la valeur MAX définie.\n", N);
            exit(0);
        }
    } else {
        printf("Usage: %s <matrice_dimension> [random seed]  \n",
               argv[0]);
        exit(0);
    }
    
    printf("\nDimension de la matrice N = %i. Seed (germe) = %d .\n", N,seed);
}

/* Initialiser A et B (et X à 0.0s) */
void initializeInputs() {
    int row, col;

    printf("\nInitialisation...\n");
    for (col = 0; col < N; col++) {
        for (row = 0; row < N; row++) {
            A[row][col] = (float)rand() / 32768.0;
        }
        B[col] = (float)rand() / 32768.0;
        X[col] = 0.0;
    }

}

void printInputs() {
    int row, col;

    if (N < 100) {
        printf("\nA =\n\t");
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                printf("%5.2f%s", A[row][col], (col < N-1) ? ", " : ";\n\t");
            }
        }
        printf("\nB = [");
        for (col = 0; col < N; col++) {
            printf("%5.2f%s", B[col], (col < N-1) ? "; " : "]\n");
        }
    }
}


void printX() {
    int row;

    if (N < 100) {
        printf("\nX = [");
        for (row = 0; row < N; row++) {
            printf("%5.2f%s", X[row], (row < N-1) ? "; " : "]\n");
        }
    }
}




void backSubstitution(){
    int norm, row, col;
    /* Back substitution */
    for (row = N - 1; row >= 0; row--) {
        X[row] = B[row];
        for (col = N-1; col > row; col--) {
            X[row] -= A[row][col] * X[col];
        }
        X[row] /= A[row][row];
    }

}


void gaussSeq() {
    int norm, row, col;  /* Normalisation et soustraction pour avoir les 0*/
    float multiplier;

    printf("Execution sequentielle ... .\n");

    for (norm = 0; norm < N - 1; norm++) {
        for (row = norm + 1; row < N; row++) {
            multiplier = A[row][norm] / A[norm][norm];
            for (col = norm; col < N; col++) {
                A[row][col] -= A[norm][col] * multiplier;
            }
            B[row] -= B[norm] * multiplier;
        }
    }
    backSubstitution();
}


