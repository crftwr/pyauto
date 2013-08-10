import os
import sys
import time

sys.path[0:0] = [
    os.path.join( os.path.split(sys.argv[0])[0], '../../..' ),
    ]

import pyauto

wnd = pyauto.Window.find( "Notepad", None )

if not wnd:
    pyauto.shellExecute( None, r"c:\windows\system32\notepad.exe" )
    while not wnd:
        wnd = pyauto.Window.find( "Notepad", None )
        time.sleep(1)

if wnd:

    rect = wnd.getRect()
    print( "rect:", rect )
    rect = list(rect)
    for i in range(100):
        rect[0] += 1
        rect[1] += 1
        wnd.setRect( rect )
        time.sleep(0.01)
    for i in range(100):
        rect[2] -= 1
        rect[3] -= 1
        wnd.setRect( rect )
        time.sleep(0.01)
    rect = wnd.getRect()
    print( "rect:", rect )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.minimize()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.restore()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.maximize()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.restore()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.maximize()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.minimize()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.restore()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

    time.sleep( 1 )

    wnd.restore()

    time.sleep( 1 )

    if wnd.isMinimized(): print( "minimized", end="" )
    if wnd.isMaximized(): print( "maximized", end="" )
    if not wnd.isMinimized() and not wnd.isMaximized() : print( "normal", end="" )
    print( "" )

