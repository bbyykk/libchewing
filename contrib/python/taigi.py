from ctypes import *
from functools import partial
import sys

_libtaigi = None
if sys.platform == "win32": # Windows
    import os.path
    # find in current dir first
    dll_path = os.path.join(os.path.dirname(__file__), "taigi.dll")
    if not os.path.exists(dll_path):
        dll_path = "taigi.dll" # search in system path
    _libtaigi = CDLL(dll_path)
else: # UNIX-like systems
    _libtaigi = CDLL('libtaigi.so.3')

_libtaigi.taigi_commit_String.restype = c_char_p
_libtaigi.taigi_buffer_String.restype = c_char_p
_libtaigi.taigi_cand_String.restype = c_char_p
_libtaigi.taigi_zuin_String.restype = c_char_p
_libtaigi.taigi_aux_String.restype = c_char_p
_libtaigi.taigi_get_KBString.restype = c_char_p


def Init(datadir, userdir):
    return _libtaigi.taigi_Init(datadir, userdir)


class TaigiContext:
    def __init__(self, **kwargs):
        if not kwargs:
            self.ctx = _libtaigi.taigi_new()
        else:
            syspath = kwargs.get("syspath", None)
            userpath = kwargs.get("userpath", None)
            self.ctx = _libtaigi.taigi_new2(
                syspath,
                userpath,
                None,
                None)

    def __del__(self):
        _libtaigi.taigi_delete(self.ctx)

    def __getattr__(self, name):
        func = 'taigi_' + name
        if hasattr(_libtaigi, func):
            wrap = partial(getattr(_libtaigi, func), self.ctx)
            setattr(self, name, wrap)
            return wrap
        else:
            raise AttributeError(name)

    def Configure(self, cpp, maxlen, direction, space, kbtype):
        self.set_candPerPage(cpp)
        self.set_maxChiSymbolLen(maxlen)
        self.set_addPhraseDirection(direction)
        self.set_spaceAsSelection(space)
        self.set_KBType(kbtype)
