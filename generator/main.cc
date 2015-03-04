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
namespace option{
  bool cpp = true;
  bool templates = true;
}
int main(int argc, char**argv)
{
    uint8_t input[102400];
    size_t inputsize;
    size_t outputsize;
    char *out;
    struct grammar *result;
    std::string filename; 
    std::string header_filename;
    NailArena arena;
    inputsize = fread(input, 1, sizeof(input), infile(argc,argv));
    //fprintf(stderr, "inputsize=%zu\ninput=", inputsize);
    //fwrite(input, 1, inputsize, stderr);  
    // print_parser_invocation(parse_parser_invocation(input,inputsize),stdout,0);
    // exit(0);
    if(std::string(argv[0]).find("cnail") != std::string::npos)
      {
        option::cpp = false;
        option::templates = false;
      }
    NailArena_init(&arena, 4096);
    result =  parse_grammar(&arena,input,inputsize);
    //    result = parse_grammar(input,inputsize);
    //    if(result)
    //      print_grammar(result, stderr,0);    exit(1);
    
     std::string headerfilename = std::string(argv[1]) + ".h";
     std::string implfilename = std::string(argv[1]) + ".c";
     if(!result){
       error("Syntax error in grammar\n");
       return 1;
     }

     try {
       if(option::templates){
         headerfilename += "h";
         implfilename += "c";
       }
       std::ofstream header(headerfilename);
       std::ofstream impl(implfilename);
       std::stringstream discard;
       if(!header.is_open())
         error("Cannot open output file %s\n",headerfilename.c_str());
       if(!impl.is_open())
         error("Cannot open output file %s\n",implfilename.c_str());
       emit_header(&header,result);
       if(!option::templates)
         impl << "#include \""<< headerfilename << "\""<<std::endl;
       if(!option::cpp)
         emit_parser(&impl,&header,result);
       else
         emit_directparser(&impl,&header,result);

       emit_generator(&impl,&header,result);
       impl << std::endl;
       header << std::endl;       
       return 0;
     } catch ( std::exception const &e){
       error("An error occured:%s\n",e.what());
       return 1;
     }
}
