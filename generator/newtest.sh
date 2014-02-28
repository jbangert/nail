#!/bin/bash -x
make && ./nail test.nail /tmp/bar.c -parser_hammer -generator && cat testharness.c >> /tmp/bar.c && astyle /tmp/bar.c && gcc -std=gnu99 -ggdb -DXYZZY=grammar /tmp/bar.c -lhammer
