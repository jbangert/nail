#include <assert.h>
#include "generator.h"

#define N_MACRO_IMPLEMENT
#include "grammar.h"
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
    //  print_struct_elem(parse_struct_elem(input,inputsize),stdout,0);
    // exit(0);
     result =  parse_grammar(input,inputsize);

    if(result) {
            print_grammar(result,stdout,0);
            return 0;
    } else {
        return 1;
    }
}
