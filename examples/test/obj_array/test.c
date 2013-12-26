
#include "test.h"

#define N_MACRO_IMPLEMENT
#include "grammar.h"
int main()
{
      
    uint8_t input[102400];
    size_t inputsize;
    const struct obj_array *result;
    inputsize = fread(input, 1, sizeof(input), stdin);
    fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    fwrite(input, 1, inputsize, stderr);  
    result =  parse_obj_array(input,inputsize);
    if(result) {
            print_obj_array(result,stdout,0);
            return 0;
    } else {
        return 1;
    }
}
