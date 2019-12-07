# -*- coding: utf_8 -*-

import os
import sys

sys.path[0:0] = [
    os.path.abspath( os.path.join( os.path.split(sys.argv[0])[0], '../../..' ) ),
    ]

import pyauto
    
def dump( wnd, indent ):
    if not wnd: return

    if wnd.isVisible() and not wnd.isMinimized():

        def Window_getProcessName(wnd):
            try:
                return wnd.getProcessName()
            except:
                return "(error)"

        print( "%s%s:%s:%s%s" % ( " " * indent, Window_getProcessName(wnd), wnd.getText(), wnd.getClassName(), wnd.getRect() ) )

        child = wnd.getFirstChild()
        
        while child:

            dump( child, indent + 4 )

            child = child.getNext()


root = pyauto.Window.getDesktop()

dump(root,0)
