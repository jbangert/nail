#include <stdio.h>
#define TOKENPASTE(x,y) x ## y
#define CAT(x,y) TOKENPASTE(x,y)
int main(){
    uint8_t input[102400];
    size_t inputsize;
    n_trace test;
    pos p;
    inputsize = fread(input, 1, sizeof(input), stdin);
    fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    fwrite(input, 1, inputsize, stderr);
    fprintf(stderr,"\n\n\n");
    n_trace_init(&test,4096,4096);
    p = CAT(packrat_,XYZZY)(NULL,&test, input,0,inputsize);
    fprintf(stderr, "pos= %d\n", p);
    fwrite(test.trace,sizeof(pos),test.iter,stdout);
}
