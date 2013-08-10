# -*- coding: utf_8 -*-

import os
import sys

sys.path[0:0] = [
    os.path.join( os.path.split(sys.argv[0])[0], '../../..' ),
    ]

import pyauto

print( 'send ABC亜意鵜絵緒' )

pyauto.Input.send([

    pyauto.KeyDown(pyauto.VK_SHIFT),

    pyauto.Key(ord('A')),
    pyauto.Key(ord('B')),
    pyauto.Key(ord('C')),

    pyauto.KeyUp(pyauto.VK_SHIFT),

    pyauto.Char(ord('亜')),
    pyauto.Char(ord('意')),
    pyauto.Char(ord('鵜')),
    pyauto.Char(ord('絵')),
    pyauto.Char(ord('緒')),

])
