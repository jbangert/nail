//Note that all nail offsets are in bits
int offsetsize_parse(NailArena *tmp,NailStream *str_filestream,NailStream *contentstream,uint32_t* offset,uint32_t* filesize){
        if(*offset > contentstream->size)
                return -1;
        if(*filesize > contentstream->size  - *offset)
                return -1;
        str_filestream->data = contentstream->data + *offset;
        str_filestream->pos  = 0;
        str_filestream->size = *filesize;
        str_filestream->bit_offset = 0;
        return 0;
}

int splitzip_parse(NailArena *tmp,NailStream *str_header,NailStream *str_content,NailStream *current){
        const int headersize = 1024;
        if(current->size <= headersize)
                return -1;
        if(current->pos != 0)
                return -1;
        str_header->data = current->data + current->size - headersize;
        str_header->pos = 0;
        str_header->size = headersize;
        str_header->bit_offset = 0;
        str_content->data = current->data;
        str_content->pos = 0;
        str_content->size = current->size - headersize;
        current->pos = current->size; //We consumed all of current. 
        return 0;
}
