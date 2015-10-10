
import os
import sys

sys.path[0:0] = [
    os.path.join( os.path.split(sys.argv[0])[0], '../../..' ),
    ]

import pyauto

from PIL import Image


root = pyauto.Window.getDesktop()

image = root.getImage()

print( "save desktop image as capture.png" )
print( "  size:", image.getSize() )
print( "  mode:", image.getMode() )

pil_image = Image.fromstring( image.getMode(), image.getSize(), image.getBuffer() )
pil_image.save( "capture.png" )

if 0:
    pil_icon = Image.open( "vcicon.png" ).convert("RGB")
    icon = pyauto.Image.fromString( pil_icon.mode, pil_icon.size, pil_icon.tobytes() )

    icon_pos = image.find( icon )
    print( "icon_pos:", icon_pos )
