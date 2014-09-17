struct zip{
  end_of_directory contents;
};
struct end_of_directory{
  uint16_t disks;
  uint16_t directory_disk;/*...*/
  struct {
    dir_fileheader* elem;
    size_t count;
  } files;
};
struct dir_fileheader{
  struct {
    uint8_t*elem;
    size_t count;
  } filename;
  fileentry contents;
};/*..*/
//The programmer calls these generated functions.
int gen_zip(NailArena *tmp_arena,NailStream *out,zip * val);
zip*parse_zip(NailArena *arena, 
              const uint8_t *data, size_t size);
//The programmers implements these two transformation
//functions and two similar ones for output.
extern int zip_eod_parse(NailArena *tmp,
 NailStream *filestream, NailStream *,NailStream *current);
extern int zip_compression_parse(NailArena *tmp,
 NailStream *uncomp,NailStream *compressed,uint32_t* size);
