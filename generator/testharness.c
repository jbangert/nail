#include <stdio.h>
extern HAllocator system_allocator;
#define TOKENPASTE(x,y) x ## y
#define CAT(x,y) TOKENPASTE(x,y)
int main(){
        FILE * dump = popen("od -t d4","w");
    uint8_t input[102400];
    size_t inputsize;
    n_trace test;
    pos p;
    inputsize = fread(input, 1, sizeof(input), stdin);
    fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    fwrite(input, 1, inputsize, stderr);
    fprintf(stderr,"\n\n\n");
    n_trace_init(&test,4096,4096);
    inputsize *= 8;
#if XYCONST > 0
    p = CAT(packrat_,XYZZY)(input,0,inputsize);
#else 
            p = CAT(packrat_,XYZZY)(NULL,&test, input,0,inputsize);
       
#endif
    
    fprintf(stderr, "pos= %d\n", p);
    fwrite(test.trace,sizeof(pos),test.iter,dump);
    fclose(dump);
    
#if XYCONST == 0    
    if(p==inputsize){
            XYZZY object;
            HBitWriter *bw;
            pos *tr = test.trace;
            const char *buf;
            size_t siz;
            pos succ = CAT(bind_,XYZZY)(&object,input, 0, &tr, test.trace);
            bw= h_bit_writer_new(&system_allocator);
            if(succ<0)
                    exit(-2);
             CAT(gen_,XYZZY)(bw,&object);
            buf = h_bit_writer_get_buffer(bw,&siz);
            printf("Output:\n");
            fwrite(buf,1,siz,stdout);
            printf("\nEnd of output;\n");
    }   
#endif
    if(p!= inputsize )
            exit(-1);
    else
            exit(0);
}
