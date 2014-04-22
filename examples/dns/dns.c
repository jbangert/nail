#include "dns.nail.h"
int dnscompress_parse(NailArena *tmp,NailStream *str_decompressed, NailStream *current){
        if(current->bit_offset) return -1;
        size_t begin_labels = current->pos;
        size_t pos = begin_labels;
        int first_label = 1;
        //First, figure out how large the stream should be.
        size_t size =0;
        while(1){
                uint8_t highbyte;
                if(pos >= current->size) return -1;
                size++;
                highbyte = current->data[pos++];
                if(highbyte == 0){
                        if(first_label){
                                first_label = 0;
                                current->pos = pos;
                        }
                        break;
                }
                if(highbyte >= 192){
                        if(pos >= current->size) return -1;
                        if(first_label){
                                first_label = 0;
                                current->pos = pos+1;
                        }
                        pos = highbyte & 63 << 8 | current->data[pos];
                } else {
                        size += highbyte;
                        pos += highbyte;
                }                
        }
        str_decompressed->data = n_malloc(tmp,size);
        if(!str_decompressed->data) return -1;
        str_decompressed->pos = 0;
        str_decompressed->bit_offset = 0;
        str_decompressed->size = size;
        pos = begin_labels;
        while(1){
                uint8_t highbyte;
                if(pos >= current->size) return -1;
                highbyte = current->data[pos];
                if(highbyte >= 192){                     
                        if(pos >= current->size) return -1;
                        pos = highbyte & 63 << 8 | current->data[pos+1];
                        continue;
                }
                memcpy(str_decompressed->data + str_decompressed->pos, current->data + pos, highbyte + 1);
                str_decompressed->pos += highbyte+1;
                pos += highbyte +1;
                if(highbyte == 0){
                        break;
                }
        }
        str_decompressed->pos = 0;
        return 0;
}

int dnscompress_generate(NailArena *tmp,NailStream *str_compressed, NailStream *current){
        if(NailOutStream_grow(current, 8*str_compressed->pos)) return -1;
        memcpy(current->data + current->pos, str_compressed->data, str_compressed->pos);
        current->pos += str_compressed->pos;
        return 0;
}
