#include <sys/socket.h>
#include <netinet/in.h>
#include <err.h>
#include <string.h>



#include "parser.h"
#define TYPE_CNAME 5
#define TYPE_A 1
#define TYPE_NS 2
#define TYPE_MX 15
#define QTYPE_ANY 255

extern HAllocator system_allocator;
#define narray_alloc(arr, aren, cnt) arr.count = cnt; arr.elem= n_malloc(aren,cnt * sizeof(arr.elem[0]))
#define narray_string(arr,string) arr.count = strlen(string); arr.elem = string;

#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)
