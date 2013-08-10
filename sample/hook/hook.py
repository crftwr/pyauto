# -*- coding: utf_8 -*-

import os
import sys

sys.path[0:0] = [
    os.path.join( os.path.split(sys.argv[0])[0], '../../..' ),
    ]

import pyauto

hook = pyauto.Hook()

def onKeyDown( vk, scan ):
    print( "onKeyDown : ", vk )

def onKeyUp( vk, scan ):
    print( "onKeyUp : ", vk )

def onClipboardChanged():
    print( "onClipboardChanged" )

hook.keydown = onKeyDown
hook.keyup = onKeyUp
hook.clipboard = onClipboardChanged

pyauto.messageLoop()

