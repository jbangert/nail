#!/bin/bash -x
make && ./nail test.nail /tmp/bar.c -parser_hammer && cat testharness.c >> /tmp/bar.c && astyle /tmp/bar.c && gcc -ggdb -DXYZZY=grammar /tmp/bar.c
