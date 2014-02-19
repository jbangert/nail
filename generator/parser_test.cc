#include <assert.h>
#include <stdint.h>
#include "nailtool.h"
#include <stdio.h>

#define CAT(x,y) TOKENPASTE(x,y)
int main()
{
      
    uint8_t input[102400];
    size_t inputsize;
    size_t outputsize;
    char *out;
    const struct grammar *result;
    inputsize = fread(input, 1, sizeof(input), stdin);
    fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    fwrite(input, 1, inputsize, stderr);
    X * test =CAT(parse_trace_,X)(input,inputsize,stderr,"HAMMER>");
    if(!test)
      exit(-1);
    CAT(print_,X) ( test,stdout,0);
    exit(0);
}
