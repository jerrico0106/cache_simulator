#include <stdio.h>
#include <stdint.h>

int A[64][64];
int B[64][64];
int C[64][64];

int main(){////模擬 C[i][j] = C[i][j] + A[i][k]*B[k][j]
    for(int i=0;i<64;i++){
        for(int k=0;k<64;k++){
            for(int j=0;j<64;j++){
                printf("R %lx\n",(uint64_t)&A[i][k]);
                printf("R %lx\n",(uint64_t)&B[k][j]);
                printf("R %lx\n",(uint64_t)&C[i][j]);
                printf("W %lx\n",(uint64_t)&C[i][j]);
            }
        }
    }
}
