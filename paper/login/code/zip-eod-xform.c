int zip_end_of_directory_parse(
  NailArena *tmp, NailStream *out_files,
  NailStream *out_dir, NailStream *in_current);
int zip_end_of_directory_generate(
  NailArena *tmp, NailStream *in_files,
  NailStream *in_dir, NailStream *out_current);
