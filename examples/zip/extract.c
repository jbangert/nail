#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "zip.nail.h"
#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)
char *mmap_file(char *filename, size_t *size){
        struct stat stat;
        int fil  = open(filename, O_RDONLY);
        if(fil < 0 ) return NULL;
        if(fstat(fil,&stat) < 0) return NULL;
        char *retval = mmap(NULL,stat.st_size,PROT_READ, MAP_SHARED, fil, 0);
        if(retval == MAP_FAILED) return NULL;
        *size = stat.st_size;
        close(fil);
        return retval;                
}
int main(int argc,char **argv){
        for(int f=1;f<argc;f++){
                NailArena arena;
                zip_file *zip;
                size_t size;
                jmp_buf err;
                char *filedata = mmap_file(argv[f],&size);
                if(!filedata){fprintf(stderr,"Err: Cannot open %s %s\n",argv[f], strerror(errno)); continue;}
                if(0!=setjmp(err)){
                        printf("OOM\n");
                        exit(-1);
                }
                NailArena_init(&arena,1024, &err);
                zip = parse_zip_file(&arena,filedata,size);
                if(!zip){
                        fprintf(stderr,"Err: Invalid ZIP file %s\n",argv[f]); 
                } else {
                        FOREACH(file, zip->contents.files){
                                char name[255];
                                char dirname[255]="out-";
                                strncpy(dirname+4,argv[f],sizeof(dirname)-4);
                                mkdir(dirname,0777);
                                snprintf(name,sizeof(name), "%s/%.*s",dirname,file->filename.count, file->filename.elem);
                                printf("%s = %d\n", name, file->contents.contents.count);
                                FILE *f = fopen(name,"w");
                                fwrite(file->contents.contents.elem,file->contents.contents.count,1,f);
                                fclose(f);
                        }
                }
                NailArena_release(&arena);
                munmap(filedata,size);
        }
}
