#include "zip.nail.h"
#include <zlib.h>
#define depend_parse(type) \
        int type ## _depend_parse(NailArena *foo,type ##_t *left, type ##_t *right){ \
                if(*left != *right) return -1;                      \
                else                return 0;                       \
        }                                                           \
        int type ## _depend_generate(NailArena *foo,type  ##_t *left, type  ##_t*right){  \
        *left = *right;                                             \
        return 0;                                                   \
        }

depend_parse(uint16)          
depend_parse(uint32)

int size_u32_parse(NailArena *foo, NailStream *fragment, NailStream *current, uint32_t *size){
        if(current->pos + *size > current->size)
                return -1;
        fragment->data = current->data + current->pos;
        fragment->bit_offset = current->bit_offset;
        fragment->size = *size;
        fragment->pos = 0;
        return 0;
}
int size_u32_generate(NailArena *tmp, NailStream *fragment, NailStream *current, uint32_t *size){ 
        //TODO: Refactor into a stream append library!
        if(NailOutStream_grow(current, 8*fragment->pos)<0)
                return -1;
        if(current->bit_offset)
                return -1;
        memcpy(current->data + current->pos, fragment->data, fragment->pos);
        current->pos += fragment->pos;
        *size = fragment->pos;
        return 0;
}
int offset_u32_parse(NailArena *tmp, NailStream *fragment, NailStream *current, uint32_t *offset){
        if(*offset> current->size) return -1;
        fragment->data = current->data + *offset;
        fragment->bit_offset = 0;
        fragment->pos = 0;
        fragment->size = current->size - *offset;
        return 0;
}
int offset_u32_generate(NailArena *tmp, NailStream *fragment, NailStream *current, uint32_t *offset)
{
        if(NailOutStream_grow(current,8*fragment->pos)<0)
                return -1;
        if(current->bit_offset)
                return -1;
        memcpy(current->data + current->pos, fragment->data, fragment->pos);
        *offset = current->pos;
        current->pos += fragment->pos;
        return 0;
}

int zip_compression_parse(NailArena *tmp, NailStream *uncompressed, NailStream *compressed, uint16_t *compression_method, uint32_t *uncompressed_size){
        switch(*compression_method){
        case 0:
                if(*uncompressed_size != compressed->size)
                        return -1;
                *uncompressed = *compressed; //TODO: do decompression here
                break;
        case 8:{
                z_stream strm;
                strm.zalloc = strm.zfree = strm.opaque = Z_NULL;
                strm.avail_in = compressed->size;
                strm.next_in = compressed->data;
                if(inflateInit2(&strm,-MAX_WBITS) != Z_OK)
                        return -1;
                uncompressed->data = n_malloc(tmp,*uncompressed_size);
                uncompressed->pos = 0;
                uncompressed->bit_offset = 0;
                uncompressed->size = *uncompressed_size;
                strm.next_out =  uncompressed->data;
                if(!strm.next_out) 
                        return -1;
                strm.avail_out = *uncompressed_size;
                int ret = inflate(&strm,Z_FINISH);
                if(ret != Z_STREAM_END || strm.avail_in != 0){
                        (void)inflateEnd(&strm);
                        return -1;
                }
                inflateEnd(&strm);
        }
                break;
        default:
                return -1;
        }
        return 0;
}
int zip_compression_generate(NailArena *tmp, NailStream *uncompressed, NailStream *compressed, uint16_t *compression_method, uint32_t *uncompressed_size){
        *compressed = *uncompressed;
        *compression_method = 0 ;
        *uncompressed_size = compressed->pos;
        return 0;
}
static int is_valid_directory(uint8_t *begin, uint8_t *end){
        if(end - begin<= 21 ) return 0;
        if(*(uint32_t *)begin != 0x06054b50 ) return 0;
        if(*(uint16_t *)(begin+20) + 22 != end-begin) return 0;
        return 1;
}
int zip_end_of_directory_parse(NailArena *tmp, NailStream *filestream, NailStream *directory, NailStream *entire_file){
        // http://stackoverflow.com/questions/8593904/how-to-find-the-position-of-central-directory-in-a-zip-file
        if(entire_file->bit_offset) return -1;
        uint8_t *end = entire_file->data + entire_file->size;
        uint8_t *ch = end -22;
        for(;ch>=entire_file->data && ch>= end-64*1024;ch--){
                if(is_valid_directory(ch,end)){
                        //TODO: put splitting into a separate funciton
                        size_t off = ch - entire_file->data;
                        filestream->data = entire_file->data;
                        filestream->bit_offset = 0;
                        filestream->size = off;
                        filestream->pos = 0;
                        directory->data = entire_file->data + off;
                        directory->bit_offset = 0;
                        directory->size = entire_file->size - off;
                        directory->pos = 0;
                        entire_file->pos = entire_file->size;
                        return 0;
                }
        }
        return -1;
}

int zip_end_of_directory_generate(NailArena *tmp,NailStream *filestream, NailStream *directory, NailStream *entire_file){
        //TODO: just append both to current? 
        *entire_file = *filestream;
        if(NailOutStream_grow(entire_file,8*directory->pos)) return -1;
        memcpy(entire_file->data + entire_file->pos, directory->data, directory->pos);
        entire_file->pos += directory->pos;
        return 0;
}

int crc_32_parse(NailArena *tmp_arena,NailStream *str_uncompressed,uint32_t* crc32_val){
        if(crc32(0L,str_uncompressed->data,str_uncompressed->size) != *crc32_val)
                return -1;
        else
                return 0;
}
int crc_32_generate(NailArena *tmp_arena,NailStream *str_uncompressed,uint32_t* crc32_val){
        *crc32_val = crc32(0L,str_uncompressed->data,str_uncompressed->pos);
        return 0;
}
