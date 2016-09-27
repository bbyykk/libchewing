#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import sys
import re

def tone_to_LoMaJi(s):
    finals = [ 'a' , 'e', 'i', 'o', 'u', 'm' ]
    final_dict = { 'a':  [ 'a', 'a', 'á', 'à', 'a', 'â', 'ǎ', 'ā', 'a̍', 'a̋' ],
                   'e':  [ 'e', 'e', 'é', 'è', 'e', 'ê', 'ě', 'ē', 'e̍', 'e̋' ],
                   'i':  [ 'i', 'i', 'í', 'ì', 'i', 'î', 'ǐ', 'ī', 'i̍', 'i̋' ],
                   'o':  [ 'o', 'o', 'ó', 'ò', 'o', 'ô', 'ǒ', 'ō', 'o̍', 'ő' ],
                   'u':  [ 'u', 'u', 'ú', 'ù', 'u', 'û', 'ǔ', 'ū', 'u̍', 'ű' ],
                   'm':  [ 'm', 'm', 'ḿ', 'm̀', 'm', 'm̂', 'm̆', 'm̄', 'm̍', 'm' ],
                   'ng': [ 'ng', 'ng', 'ńg', 'ǹg', 'ng', 'n̂g', 'n̆g', 'n̄g', 'n̍g', 'ng' ]
                   }
    final_dict_capital = { 'a': [ 'A', 'A', 'Á', 'À', 'A', 'Â', 'Ǎ', 'Ā', 'A̍', 'A̋' ],
                           'e': [ 'E', 'E', 'É', 'È', 'E', 'Ê', 'Ě', 'Ē', 'E̍', 'E' ],
                           'i': [ 'I', 'I', 'Í', 'Ì', 'I', 'Î', 'Ǐ', 'Ī', 'I̍', 'I' ],
                           'o': [ 'O', 'O', 'Ó', 'Ò', 'O', 'Ô', 'Ǒ', 'Ō', 'O̍', 'Ő' ],
                           'u': [ 'U', 'U', 'Ú', 'Ù', 'U', 'Û', 'Ǔ', 'Ū', 'U̍', 'Ű' ],
                           'm': [ 'M', 'M', 'Ḿ', 'M̀', 'M', 'M̂', 'M̆', 'M̄', 'M̍', 'M' ],
                           'ng':[ 'Ng', 'Ng', 'Ńg', 'Ǹg', 'Ng', 'N̂g', 'N̆g', 'N̄g', 'N̍g', 'Ng']
                   }
    for f in finals:
        a_re = re.compile(".*" + f + ".*[1-9]")
        aa = re.findall(a_re, s)
        if aa:
            a_sr = re.compile(f)
            aa_0 = aa[0]
            tone_num = int(aa_0[-1])
            # Remove the last tone number
            aa_0 = aa_0[:-1]
            if aa_0[0] == f:
                r_capital = re.sub(a_sr, final_dict_capital[f][tone_num], aa_0, 1)
                r = re.sub(a_sr, final_dict[f][tone_num], aa_0, 1)
                return (r, r_capital)
            else:
                r = re.sub(a_sr, final_dict[f][tone_num], aa_0, 1)
                return (r, r[0].upper() + r[1:])
    return None, None

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



for i in phones:
    normal, title = tone_to_LoMaJi(i)
    if normal:
        print(i + " " + normal)
        print(i + " " + title)
