import os
import sys
import subprocess
import shutil
import zipfile
import hashlib

DIST_DIR = "dist/pyauto"
DIST_SRC_DIR = "dist/src"

PYTHON_DIR = "c:/Python35"
PYTHON = PYTHON_DIR + "/python.exe"
SVN_DIR = "c:/Program Files/TortoiseSVN/bin"
DOXYGEN_DIR = "c:/Program Files/doxygen"

def unlink(filename):
    try:
        os.unlink(filename)
    except OSError:
        pass

def makedirs(dirname):
    try:
        os.makedirs(dirname)
    except OSError:
        pass

def rmtree(dirname):
    try:
        shutil.rmtree(dirname)
    except OSError:
        pass

def createZip( zip_filename, items ):
    z = zipfile.ZipFile( zip_filename, "w", zipfile.ZIP_DEFLATED, True )
    for item in items:
        if os.path.isdir(item):
            for root, dirs, files in os.walk(item):
                for f in files:
                    f = os.path.join(root,f)
                    print( f )
                    z.write(f)
        else:
            print( item )
            z.write(item)
    z.close()

DIST_FILES = [
    "pyauto",
    ]

def build():
    doc()
    exe()

def exe():
    makedirs( DIST_DIR )
    shutil.copy( "pyautocore.pyd", DIST_DIR )
    shutil.copy( "__init__.py", DIST_DIR )
    shutil.copy( "pyauto_const.py", DIST_DIR )
    shutil.copy( "pyauto_input.py", DIST_DIR )
    rmtree( DIST_DIR + "/sample" )
    shutil.copytree( "sample", DIST_DIR + "/sample", ignore=shutil.ignore_patterns(".svn","*.pyc","*.pyo") )
    rmtree( DIST_DIR + "/doc" )
    shutil.copytree( "doc/html", DIST_DIR + "/doc", ignore=shutil.ignore_patterns(".svn") )

    if 1:
        os.chdir("dist")
        createZip( "pyauto.zip", DIST_FILES )
        os.chdir("..")
    
    fd = open("dist/pyauto.zip","rb")
    m = hashlib.md5()
    while 1:
        data = fd.read( 1024 * 1024 )
        if not data: break
        m.update(data)
    fd.close()
    print( "" )
    print( m.hexdigest() )

def rebuild():
    clean()
    build()

def clean():
    rmtree("doc/html")
    unlink( "tags" )

def doc():
    rmtree( "doc/html" )
    makedirs( "doc/html" )
    subprocess.call( [ DOXYGEN_DIR + "/bin/doxygen.exe", "doc/doxyfile" ] )
    subprocess.call( [ PYTHON, "tool/rst2html_pygments.py", "--stylesheet=tool/rst2html_pygments.css", "doc/index.txt", "doc/html/index.html" ] )
    shutil.copytree( "doc/image", "doc/html/image", ignore=shutil.ignore_patterns(".svn","*.pdn") )

if len(sys.argv)<=1:
    target = "build"
else:
    target = sys.argv[1]

eval( target + "()" )

