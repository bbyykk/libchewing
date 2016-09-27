#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import sys
import re

try:
    filename = sys.argv[1]
except:
    print("Usage: " + sys.argv[0] + " path/to/input/file")
    raise SystemExit

with open(filename, 'rb') as phonefile:
    all_lines = phonefile.readlines()

phones=[]
for l in all_lines:
    ph = l.strip().split(' ')[0]
    if ph not in phones and ph[0] != '%':
        phones.append(ph)

a_re = re.compile('.*a.*[1-9]')
a_sr = re.compile('a')
for i in phones:
    aa = re.findall(a_re, i)
    if aa:
        aa_0 = aa[0]
        if aa_0[-1] == "2":
            r = re.sub(a_sr, "á", aa_0)
            r = r[:-1]
            print(aa_0 + ' ' + r)

