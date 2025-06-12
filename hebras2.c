#include <sthread.c>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>


static int n,m =2, 2;
static int A[2][2] = {{1,4},{5,6}};
static int B[2][2] = {{5,0},{3,2}};

void sumar(int i){
    int suma = 0;
    for (int j = 0; j < m; j++){
        suma += A[i][j] + B[i][j];
    }
    sthread_exit(suma);
}

static int t[n];

int main(int argc, char *argv[]){
    
    for (int i = 0; i < n; i++){
        sthread_create(&t[i], &sumar, i);
    }

    for (int i = 0; i < n; i++){
        int result = sthread_join(t[i]);
        printf("La suma de la fila %d es: %d\n", i, result);
    }

    return 0;
}