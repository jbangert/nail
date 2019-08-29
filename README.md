Nail
====

**Moved to https://github.com/hcoona/one/tree/master/nail**

\textit{Nail} is an interface generator that allows programmers
to safely parse and generate protocols defined by a Parser-Expression
based grammar. Nail also provides solutions for parsing common patterns such as
length and offset fields within binary formats that are hard to process
with existing parser generators.

Installation & Pre-requisites
=============================
The code generator (in /generator/) requires a C++ compiler and depends on boost.
The code generator can either be invoked as ./nail foo.nail , which case it will emit C++ code (in foo.nail.cc and foo.nail.hh) , or as ./cnail foo.nail in which case it will emit a two-pass parser in C (in foo.nail.c and foo.nail.h) . Generated code has no dependencies (and does not even use the C++ or C standard library).
longjmp.h/setjmp.h is used  for handling out of memory errors. 

Examples
========
Several examples are provided, pull requests for more are very welcome!
* /dns - DNS server and resolver implemented in C. Used for older benchmarks
* /dns_cpp - DNS server implemented in C++, used for newer benchmarks
* /zip - ZIP extractor,compressor in C.
* /protozip - Simplified 'pretend' zip, easier to grok
* /utf16 - mini grammar for UTF-16 -- good starting point

Network stack
=============
Nail has a (very prototype-y) network stack. Find it in /network/. The network stack depends on boost and (soon) Intel TBB and DPDK .



