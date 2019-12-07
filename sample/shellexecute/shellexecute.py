import os
import sys
import time

sys.path[0:0] = [
    os.path.abspath( os.path.join( os.path.split(sys.argv[0])[0], '../../..' ) ),
    ]

import pyauto

pyauto.shellExecute( None,
                     r"c:\windows\system32\notepad.exe",
                     r"",
                     r"",
                     None )
