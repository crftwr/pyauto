# -*- coding: utf_8 -*-

import os
import sys

sys.path[0:0] = [
    os.path.abspath( os.path.join( os.path.split(sys.argv[0])[0], '../../..' ) ),
    ]

from keymap_hook import *
from keymap_config import *

if __name__ == "__main__":

    import pyauto

    loadConfigFile()

    print( "-- keymap start --" )

    pyauto.messageLoop()

    print( "-- keymap end --" )
