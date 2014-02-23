#ifndef NAILTOOL_H
#define NAILTOOL_H
#include "expr.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cstdlib>
extern "C"{
#include <hammer/hammer.h>
#include <hammer/glue.h>

#include "grammar.h"
}

typedef struct expr{
        struct expr *parent;
        const char *str;
        size_t len;
        int pointer;
} expr;

#define FOREACH(val,coll) for(__typeof__((coll).elem[0]) *val=(coll).elem;val<(coll).elem + (coll).count;val++)
extern void emit_generator(std::ostream *out,grammar *grammar, const char *header);
extern void emit_parser(std::ostream *out,grammar *grammar);
#endif
