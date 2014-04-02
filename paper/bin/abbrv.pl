#!/usr/bin/perl
# Detects potential abbreviations at the end of a sentence.

while (<>) {
    s/%.*//;
    next unless s/([A-Z]+\.)/\e[7m$1\e[m/g;

    s/^([^\e]*\n)+//mg;
    s/^/$ARGV: /mg;
    print;
}
