import os
import sys
import time

sys.path[0:0] = [
    os.path.abspath( os.path.join( os.path.split(sys.argv[0])[0], '../../..' ) ),
    ]

import pyauto

wnd = pyauto.Window.find( "Notepad", None )

if not wnd:
    pyauto.shellExecute( None, r"c:\windows\system32\notepad.exe" )
    while not wnd:
        wnd = pyauto.Window.find( "Notepad", None )
        time.sleep(1)

if wnd:

    if 0:

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_MAXIMIZE' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_MAXIMIZE )

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_MINIMIZE' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_MINIMIZE )

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_RESTORE' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_RESTORE )

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_CLOSE' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_CLOSE )


    elif 0:

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_SIZE' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_SIZE )

        print( 'WM_SYSCOMMAND, SC_MOVE' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_MOVE )

        print( 'WM_SYSCOMMAND, SC_CONTEXTHELP' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_CONTEXTHELP )

        print( 'WM_SYSCOMMAND, SC_NEXTWINDOW' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_NEXTWINDOW )


    elif 0:

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_MOUSEMENU' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_MOUSEMENU )

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_KEYMENU' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_KEYMENU )

    elif 1:

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_TASKLIST' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_TASKLIST )

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_SCREENSAVE' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_SCREENSAVE )

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_MONITORPOWER' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_MONITORPOWER, 2 )


    elif 0:

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_HOTKEY' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_HOTKEY )

        time.sleep(1)

        print( 'WM_SYSCOMMAND, SC_DEFAULT' )
        wnd.sendMessage( pyauto.WM_SYSCOMMAND, pyauto.SC_DEFAULT )

