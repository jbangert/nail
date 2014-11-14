zip = {   /*simplified and cut for brevity*/
    /* Call zip_eod transform to isolate end_directory and contents streams*/
     $contents, $end_directory transform zip_eod ($current)
     /* Parse end_directory stream. The end_of_directory parser takes the 
      content stream as a parameter */
     contents apply $end_directory end_of_directory($contents)
}
end_of_directory($filestream) = { /*Grammar rules can have parameters*/
     uint32 = 0x06054b50 /* constant*/
     disks uint16 | [0]  /* constraint*/
     directory_disk uint16 | [0]
     @this_records uint16 /*dependent field*/
     @total_records uint16 
     /*The following transform ensures these two fields are always equal*/
     transform uint16_depend (@this_records @total_records)
     @directory_size uint32 /*These two dependent fields are used by the  */
     @directory_start uint32/* transformations below to find the directory*/
     /*dirstr1 is the the suffic of filestream starting @directory_start*/
     $dirstr1 transform offset_u32 ($filestream @directory_start) 
     /*Another stream with @directory_size bytes starting at that offset*/
     $directory_stream transform size_u32 ($dirstr1 @directory_size)
     @comment_length uint16 /*Dependent field used with the built in n_of*/
     comment n_of @comment_length uint8 /* Variable-length comment*/
     files apply $directory_stream n_of @total_records 
        dir_fileheader($filestream) /*Array of directory entries*/
}
dir_fileheader($filestream) = { 
     uint32 = 0x02014b50
     @compressed_size uint16 
     @crc32 uint32 
     @file_name_len uint16
     @off uint32
     filename n_of @file_name_len uint8
     $cstream transform offset_u32 ($filestream @off)
     contents apply $cstream fileentry(@crc32,@compressed_size)
}
