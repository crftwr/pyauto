# -*- coding: utf_8 -*-

import os
import sys

sys.path[0:0] = [
    os.path.join( os.path.split(sys.argv[0])[0], '../../..' ),
    ]

import pyauto
    
print( pyauto.Window.getMonitorInfo() )

