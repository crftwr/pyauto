# -*- coding: utf_8 -*-

import os
import sys

sys.path[0:0] = [
    os.path.abspath( os.path.join( os.path.split(sys.argv[0])[0], '../../..' ) ),
    ]

import pyauto

wnd_found = None
def for_each_window( wnd, arg ):
    global wnd_found
    if wnd.getText().find("メモ帳") >= 0:
        wnd_found = wnd
        return False
    else:
        return True

pyauto.Window.enum( for_each_window, None )

if wnd_found:
    wnd_found.setRect( (0,0,640,480) )

