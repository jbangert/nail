#define TOKENPASTE(x, y) x ## y  //http://stackoverflow.com/questions/1597007
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)


//FIXME: broken
#define N_FOREACH(type, name, container) size_t name ## _iter; type *name; for(;(name ##_iter < (container).count) && (name = (container).elem[0]); name = (container).elem[name##_iter],name ## _iter++)


