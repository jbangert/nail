#!/usr/bin/env ruby
offset = 0
out = ""

header = [ARGV.size].pack("S>")
ARGV.each do |file|
  contents = File.read(file)
  out+= contents
  filesiz = contents.bytesize
  header += [offset, filesiz].pack("L>L>") + file+ [0].pack("C")
  offset = out.bytesize()
end
while header.bytesize()<1024
  header += [0].pack("C")
end

print out
print header

