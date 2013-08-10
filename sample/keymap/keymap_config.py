# -*- coding: utf_8 -*-

user_namespace = {}

def loadConfigFile():

    filepath = 'config.py'

    try:
        print( "-- keymap config --" )
        fd = open( filepath, 'r' )
        fileimage = fd.read()
    except IOError:
        print( 'ERROR : no config file.' )

    try:
        code = compile( fileimage, filepath, 'exec' )
        exec( code, user_namespace, user_namespace )
    except SyntaxError as e:
        print( e )
