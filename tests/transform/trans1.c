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
int offsetsize_generate(NailArena *tmp, NailStream *str_filestream, NailStream *contentstream, uint32_t *offset, uint32_t *filesize){
        //Append ilestream to contentstream
        if(parser_fail(NailOutStream_grow(contentstream,str_filestream->pos << 3)))
                return -1;
        if(contentstream->bit_offset || str_filestream->bit_offset)
                return -1;
        memcpy(contentstream->data+contentstream->pos, str_filestream->data, str_filestream->pos);
        *offset = contentstream->pos; // TODO: check for overflows
        contentstream->pos += str_filestream->pos;
        *filesize = str_filestream->pos;
        return 0;
}
static const int headersize = 1024;
int splitzip_parse(NailArena *tmp,NailStream *str_header,NailStream *str_content,NailStream *current){
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
int splitzip_generate(NailArena *tmp, NailStream *str_header, NailStream *str_content, NailStream *current){
        //TODO: Find a better way to guarantee header length of 1024. Do a transform for padding?
        if(str_header->pos > 1024)
                return -1;
        if(parser_fail(NailOutStream_grow(current, str_content->pos + headersize)))
                return -1;
        memcpy(current->data + current->pos, str_content->data, str_content->pos);
        current->pos += str_content->pos;
        memcpy(current->data + current->pos, str_header->data, str_header->pos);
        current->pos += str_header->pos;
        memset(current->data + current->pos,0,headersize - str_header->pos);
        current->pos += headersize - str_header->pos;
        return 0;
}
