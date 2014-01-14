#include <assert.h>
#include "test.h"

#define N_MACRO_IMPLEMENT
#include "grammar.h"
int main()
{
      
    uint8_t input[102400];
    size_t inputsize;
    size_t outputsize;
    char *out;
    const struct obj_array *result;
    inputsize = fread(input, 1, sizeof(input), stdin);
    fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    fwrite(input, 1, inputsize, stderr);  
    result =  parse_obj_array(input,inputsize);

    if(result) {
            out= obj_array_write(result,&outputsize);
            assert(outputsize == inputsize);
            assert(memcmp(out,input,outputsize));
            print_obj_array(result,stdout,0);
            return 0;
    } else {
        return 1;
    }
}
