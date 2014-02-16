#include "nailtool.h"
#include <cstdio>
#include <fstream>
#define error(...) {fprintf(stderr,__VA_ARGS__); exit(-1);}
FILE *infile(int argc,char **argv){
        char commandbuffer[1024];
        FILE *infile;
        if(argc<2){
                fprintf(stderr,"Usage %s <grammar file> <options>\n", argv[0]);
                exit(-2);
        }
        snprintf(commandbuffer,sizeof commandbuffer,"cpp -I/usr/include/nail/generator < \"%s\" |sed '/^\\#/d'",argv[1]);
        infile = popen(commandbuffer,"r");
        if(!infile) error ("Cannot open pipe\n");
        return infile;
}
int main(int argc, char**argv)
{
    uint8_t input[102400];
    size_t inputsize;
    size_t outputsize;
    char *out;
    struct grammar *result;
    inputsize = fread(input, 1, sizeof(input), infile(argc,argv));
    //fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    //fwrite(input, 1, inputsize, stderr);  
    // print_parser_invocation(parse_parser_invocation(input,inputsize),stdout,0);
    // exit(0);
    result =  parse_grammar(input,inputsize);
    print_grammar(result, stderr,0);
    exit(1);
     if(result) {
       std::ofstream *outfile = NULL;
       int i =0;
       for(i=2;i<argc;i++){
         if(argv[i][0] != '-'){
           if(outfile)
             delete outfile;
           outfile = new std::ofstream(argv[i]);
           if(!outfile || !outfile->is_open())
             error("Cannot open output file %s\n",argv[i]);
           continue;
         }
         else if(!strcmp(argv[i],"-header"))
           {
             error("Not implemented");
             //emit_header(outfile,result);
           }
         else if(!strcmp(argv[i],"-parser_hammer")){
            emit_hammer_parser(outfile,result,argv[1]);
         }
         else if(!strcmp(argv[i],"-generator"))
           emit_generator(outfile,result,argv[1]);
         else
           error("Unknown command line option %s",argv[i]);
       }
       return 0;
     } else {
       return 1;
     }
}
