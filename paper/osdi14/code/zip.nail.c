zip = {  /*cut for brevity*/
 $filestream, $end_directory transform zip_eod ($current)
 contents apply $end_directory end_of_directory($filestream)
}
end_of_directory($filestream) = { 
     uint32 = 0x06054b50
     disks uint16 | [0]
     directory_disk uint16 | [0]
     @this_records uint16
     @total_records uint16 
     transform uint16_depend (@this_records @total_records)
     @directory_size uint32 
     @directory_start uint32
     $dirstr1 transform offset_u32 ($filestream @directory_start) 
     $directory_stream transform size_u32 ($dirstr1 @directory_size)
     @comment_length uint16
     comment n_of @comment_length uint8
     files apply $directory_stream n_of @total_directory_records 
                 dir_fileheader($filestream)
}
dir_fileheader($filestream) = { 
     uint32 = 0x02014b50/*...*/
     @compressed_size uint16 /*...*/
     @crc32 uint32/*...*/
     @file_name_len uint16/*...*/
     @off uint32
     filename n_of @file_name_len uint8/*...*/
     $cstream transform offset_u32 ($filestream @off)
     contents apply $cstream fileentry(@crc32,@compressed_size/*...*/)
}
fileentry(@crc32 uint32,@size uint32/*...*/) = {
     uint32 = 0x04034b50/*...*/
     @crc_local uint32/*...*/
     $compressed transform size_u32 ($current @size)
     $uncompressed transform zip_compression ($compressed @size/*...*/)
     transform crc_32 ($uncompressed @crc32)
     contents apply $uncompressed many uint8
     transform uint16_depend (@crc_local @crc32)/*...*/
}   