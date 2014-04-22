#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "zip.nail.h"
char *mmap_file(char *filename, size_t *size){
        struct stat stat;
        int fil  = open(filename, O_RDONLY);
        if(fil < 0 ) return NULL;
        if(fstat(fil,&stat) < 0) return NULL;
        char *retval = mmap(NULL,stat.st_size,PROT_READ, 0, fil, 0);
        if(!retval) return NULL;
        *size = stat.st_size;
        close(fil);
        return retval;                
}
int main(int argc,char **argv){
        for(int f=1;f<argc;f++){
                NailArena arena;
                zip_file *zip;
                size_t size;
                char *filedata = mmap_file(argv[f],&size);
                if(!filedata){fprintf(stderr,"Err: Cannot open %s\n",filedata); continue;}
                NailArena_init(&arena,1024);
                zip = parse_zip_file(&arena,filedata,size);
                if(!zip){
                        fprintf(stderr,"Err: Invalid ZIP file %s\n",filedata); 
                } else {
                }
                NailArena_release(&arena);
                munmap(filedata,size);
        }
}
