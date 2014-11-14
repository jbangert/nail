labels = <many {@length  uint8 | 1..64
              label n_of @length uint8 }
          uint8 = 0 > 
compressed={
   $decompressed transform dnscompress($current)
   labels apply $decompressed label
}     
question={
  labels compressed
  qtype uint16 | 1..16 
  qclass uint16 | [1,255]
}
answer={
  labels compressed
  rtype uint16 | 1..16
  class uint16 | [1]
  ttl uint32
  @rlength uint16 
  rdata n_of @rlength uint8 
}
dnspacket = {
  id uint16
  qr uint1
  opcode uint4
  aa uint1 
  tc uint1
  rd uint1
  ra uint1
  uint3 = 0
  rcode uint4
  @questioncount uint16
  @answercount uint16
  @authoritycount uint16   // Ignored
  @additionalcount uint16  //Ignored       
  questions n_of @questioncount question         
  responses n_of @answercount answer
  authority n_of @authoritycount answer
  additional n_of @additionalcount answer
}
