import os
import sys
import time
import math

sys.path[0:0] = [
    os.path.abspath( os.path.join( os.path.split(sys.argv[0])[0], '../../..' ) ),
    ]

import pyauto

for i in range(1000):

    x = math.cos( i*0.01 ) * 200 + 300
    y = math.sin( i*0.01 ) * 200 + 300

    pyauto.Input.send([

        pyauto.MouseMove( int(x), int(y) ),

    ])

    time.sleep( 0.001 )

