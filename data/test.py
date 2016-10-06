#!/usr/bin/python
# -*- coding: utf-8 -*-
import os
import sys
import re

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

def is_lomaji(s):
    for a in final_dict.values():
        for l in a:
            if l in s:
                return True
    return False

s = u"ùaaa"
c = s[0]
print(c)
print(is_lomaji('ò'))
print(is_lomaji(s))
print(is_lomaji('y'))
print(is_lomaji('n'))
print(is_lomaji('ng'))
