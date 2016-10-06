#!/usr/bin/python
# -*- coding: utf-8 -*-
### Create tsi.src format from original taigime.txt
### filtering out the non-chinese output
###
import os
import sys
import re

finals = [ 'a' , 'e', 'i', 'o', 'u', 'm' ]
final_dict = { 'a':  [ 'a', 'a', 'á', 'à', 'a', 'â', 'ǎ', 'ā', 'a̍', 'a̋' ],
               'e':  [ 'e', 'e', 'é', 'è', 'e', 'ê', 'ě', 'ē', 'e̍', 'e̋' ],
               'i':  [ 'i', 'i', 'í', 'ì', 'i', 'î', 'ǐ', 'ī', 'ı̍', 'i̋' ],
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

def is_lomaji(s):
    # Get all the values from dictionary
    for a in final_dict.values():
	# Get each series
        for l in a:
	    # if value in the string, then it is
            if l in s:
                return True

    for a in final_dict_capital.values():
        for l in a:
            if l in s:
                return True
    return False



def tone_to_LoMaJi(s):
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

def split_by_num(s):
    ### Sepaprate the key in with phone
    ### Output the tone and no tone version
    ###
    alpha_num = re.compile("([a-zA-Z]+)([0-9]+)")
    words = re.findall(alpha_num, s)
#    print(words)
    with_tone = ""
    no_tone = ""
    for i in words:
        with_tone = with_tone + ''.join(i) + ' '
        no_tone = no_tone + i[0] + ' '
#    print(with_tone)
#    print(no_tone)
    return with_tone,no_tone

try:
    filename = sys.argv[1]
except:
    print("Usage: " + sys.argv[0] + " path/to/input/file")
    raise SystemExit

with open(filename, 'rb') as phonefile:
    all_lines = phonefile.readlines()

phones=[]
for l in all_lines:
    ph = l.strip().split('\t')
    if ph[1]:
        # Check the first character to screen out non-chinese
        if not ph[1][0].isalpha() and not is_lomaji(ph[1]):
            #print(ph[1] + ' ' + ph[0])
            with_tone, no_tone = split_by_num(ph[0])
            if with_tone:
                print(ph[1] + ' ' + "500 " + with_tone)

