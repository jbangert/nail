#!/usr/bin/env ruby
REPEATCOUNT=3
SUCCESSPROB=0.9
tlds = ["com","net","org"]

nailzone = File.open("naildns.zone", "w")
bindzone = File.open("bind.zone", "w") 
bindzone.write  <<-here
$ORIGIN .
$TTL 1h
. IN SOA localhost. root.benchmark. ( 2007120710 ; serial number of this zone file 
              1d         ; slave refresh (1 day)
              2h         ; slave retry time in case of a problem (2 hours)
              4w         ; slave expiration time (4 weeks)
              1h         ; maximum caching time in case of failed lookups (1 hour)
              ) 
localhost. IN A 127.0.0.1
. IN NS localhost.
here
queryperf_file = File.open("queryperf.input","w")
queryperf = []
words = File.read("/usr/share/dict/british-english").split("\n").map(&:downcase).map { |x|  x.gsub(/[^a-z]+/, '')}.uniq
domaincount=ARGV[0].to_i || 10000
print "#{domaincount}\n"
domaincount.times  { 
  domain = ""
  [1,2].sample.times {
    domain += "#{words.sample}."
  }
  domain += tlds.sample 
 # print "#{domain} \n"
  if(rand()<SUCCESSPROB)
      nailzone.write("#{domain}:A:127.0.0.1\n")
      bindzone.write("#{domain}. IN A 127.0.0.1\n")
  end
  rand(REPEATCOUNT).times {
    queryperf << "#{domain} A\n"
  }
  #TODO: write a bind zonefile
}

queryperf.shuffle.each {|x| queryperf_file.write x}
nailzone.close
queryperf_file.close
