#line 1 "test_harness.c"
#include <stdio.h>
#include <stdint.h>
#define TOKENPASTE(x,y) x ## y
#define CAT(x,y) TOKENPASTE(x,y)
#define XYZZY foo

int main(){
        //     FILE * dump = popen("od -t d4","w");
    uint8_t input[102400];
    size_t inputsize;

    NailArena arena;
    pos p;
    inputsize = fread(input, 1, sizeof(input), stdin);
    //fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
//    fwrite(input, 1, inputsize, stderr);
//    fprintf(stderr,"\n\n\n");
//    n_trace_init(&test,4096,4096);
    NailArena_init(&arena,4096);
//    inputsize *= 8;
    XYZZY *object; 
    object = CAT(parse_,XYZZY)(&arena,input,inputsize);
    if(object){
            printf("\n Successfully parsed! Trying to generate\n");
            const char *buf;
            size_t siz;
            NailStream stream;
            NailOutStream_new(&stream,4096);
            CAT(gen_,XYZZY)(&stream,object);
            buf = NailOutStream_buffer(&stream,&siz);
            printf("Output:\n");
            fwrite(buf,1,siz,stdout);
            printf("\nEnd of output;\n");
            if(siz != inputsize)
                    return -1;
    }   
    else {
            fprintf(stderr, "Could not parse\n");
            exit(-1);
    }
    return 0;
}
