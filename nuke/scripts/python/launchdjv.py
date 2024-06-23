import nuke
import os
import subprocess
DJV_VIEW_PATH = '/usr/local/djv/bin/djv_view'

def run( node ):
    result = nuke.getFramesAndViews( label =  "Frames to djv_view:", default="%s, %s" % ( node['first'].value(), node['last'].value() ), maxviews = 1 )
    if result != None:
        subprocess.call( "%s %s &" % ( DJV_VIEW_PATH, getDjvParams( node, result[0] ) ), shell=True )

def getDjvParams( node, frameRange ):
    startFrame, endFrame = frameRange.split(',')
    sourceFilePath = node['file'].value()
    filenameWithSeq = os.path.basename( sourceFilePath )
    filenameSplit = filenameWithSeq.split(".")
    if len( filenameSplit ) == 3:
        newFilename  = "%s.%s.%s" % ( filenameSplit[0], "%04d-%04d" % ( (int)(startFrame), (int)(endFrame) ), filenameSplit[-1] )
        return os.path.join( os.path.dirname( sourceFilePath ) , newFilename   )
    else:
        return None
    pass
