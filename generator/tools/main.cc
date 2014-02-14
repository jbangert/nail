#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "shared.h"
FILE *infile(int argc,char **argv){
        char commandbuffer[1024];
        FILE *infile;
        if(argc<2){
                fprintf(stderr,"%s <grammar file> \n", argv[0]);
                exit(-2);
        }
        snprintf(commandbuffer,sizeof commandbuffer,"cpp -I/usr/include/nail/generator < \"%s\" |sed '/^\\#/d'",argv[1]);
        infile = popen(commandbuffer,"r");
        if(!infile)fprintf(stderr,"Cannot open pipe\n");
        return infile;
}
int main(int argc, char **argv)
{
  FILE *f;
  f=infile(argc,argv);
  std::list<Token *>* lex = lexer (f);
  int i =0;
  void *parser = ParseAlloc(malloc);
  std::string out;
  ParseTrace(stderr,"p");
  for(auto &x : *lex){
    if(i++ % 10 == 0) printf("\n");
    //printf("%s ",tokennames[x->type]);
    Parse(parser,x->type,new std::stringstream(x->data),&out);
  }
  //Parse(parser,0,NULL,&out);
  puts(out.c_str());
  
}


