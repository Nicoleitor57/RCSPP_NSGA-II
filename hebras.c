#include <sthread.c>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

void factorial(int x){
    int result;

    if (x == 0){
        result = 1;
    } else {
        sthread_t n1;
        sthread_create(&n1, &factorial, x - 1);
        resutl = x * sthread_join(n1);

    }
    sthread_exit(result);
}

static sthread_t  t1, t2, t3;


int main(int argc, char *argv[]){
   
    sthread_create(&t1, &factorial, n);
    int a = sthread_join(t1);

    sthread_create(&t2, &factorial, r);
    int b = sthread_join(t2);

    sthread_create(&t3, &factorial, n-r);
    int c = sthread_join(t3);

    int resultado  = a / (b * c);
    printf("El resultado es: %d\n", resultado);
}
