#!/usr/bin/python
# ----------------------------------------------------------------------
# (C) 2008,2009 method 
# http://www.methodstudios.com 
# ----------------------------------------------------------------------
"""Usage:
    sendtoscratch [required args]
Ex:
    sendtoscratch --command "LOAD" --firstFrame "1" --startFrame "1" --endFrame "5" --sourceFilePath "\\\\frack.methodstudios.com\\jobs\\camptest1\\camptest1_mocha\\sequences\\MOC\\MOC001\\images\\renders\\boontest2009-layer\\boontest2009-layer_layer1.0001.exr" --project "camptest1_mocha" --clipName "boontest2009-layer_layer1" --watchdir "C:\\sendtoscratch\\method_scratch\\watch" --tempdir "C:\\sendtoscratch\\method_scratch\\jobs\\camptest1\\camptest1_mocha\\MOC\\MOC001\\boontest2009-layer_layer1" --group "Import" --desk "Tue_07-14-09" --reel "last" --imageMode "true" --layer "0" --resultFilePath "C:\\sendtoscratch\\method_scratch\\jobs\\camptest1\\camptest1_mocha\\MOC\\MOC001\\boontest2009-layer_layer1\\result.xml" --logFilePath "C:\\sendtoscratch\\method_scratch\\jobs\\camptest1\\camptest1_mocha\\MOC\\MOC001\\boontest2009-layer_layer1\\log.xml" --outputDirPath "C:\\sendtoscratch\\method_scratch\\jobs\\camptest1\\camptest1_mocha\\MOC\\MOC001\\boontest2009-layer_layer1" --shot "DefaultImport" --slotLength "0" --user "boon" --currentDate "Tue, 14 Jul 2009 18:58:46 +0000" 

Required Arguments:
    --command               SCRATCH command e.g., LOAD, ADDREF
    --firstFrame            First frame of files sequence e.g., 1
    --startFrame            Start frame e.g., 5
    --endFrame              End frame e.g., 10
    --sourceFilePath        Image file path.
    --project               Project name e.g., campagin_job
    --clipName              Clip name
    --watchdir              SCRATCH Watch directory
    --tempdir               SCRATCH Temp directory
    --group                 Group name
    --desk                  Desk name
    --reel                  Reel
    --imageMode             Image mode [default is True]
    --layer                 Layer number
    --resultFilePath        result.xml path
    --logFilePath           log.xml path
    --outputDirPath         SCRATCH output file path
    --shot                  Shot
    --shotLength            Shot length
    --user                  User name
    --currentDate           Current date and time
"""

import sys
import getopt
import os
import subprocess
import exceptions
import time
import warnings
import os.path
import re
import string
import shutil
import time
from xml.sax.handler import ContentHandler
from xml.sax import parse

# Global variables
gCommand        =   None
gFirstFrame     =   None
gStartFrame     =   None
gEndFrame       =   None
gSourceFilePath =   None
gProject        =   None
gClipName       =   None
gWatchdir       =   None
gTempdir        =   None
gGroup          =   None
gDesk           =   None
gReel           =   None
gImageMode      =   None
gLayer          =   None
gResultFilePath =   None
gLogFilePath    =   None
gOutputDirPath  =   None
gShot           =   None
gSlotLength     =   None
gAspectRatio    =   None
gUser           =   None
gCurrentDate    =   None

def main():

    #time.sleep(15)
    
    global gCommand, gFirstFrame, gStartFrame, gEndFrame, gSourceFilePath, gProject, gClipName, gWatchdir, gTempdir, gGroup, gDesk, gReel, gImageMode, gLayer, gResultFilePath, gLogFilePath, gOutputDirPath, gShot, gSlotLength, gAspectRatio, gUser, gCurrentDate


    #Get Revision everytime the code is checked in
    #If the revision is '---', it means it cannot get the revision string.
    versionStr = '$Rev: 423 $'
    p = re.compile('\\$Rev:\s*([0-9]*)\s*\\$')
    m = p.match(versionStr)
    rev = "---"
    if m:
         rev = m.group(1)

    print( "method's sendtoscratch (Revision: %s)- send nuke read node to SCRATCH command (C) 2009 method" % rev)
    print( "" )


    # parse command line options
    try:
        opts, args = getopt.getopt(sys.argv[1:], "h:", ["help", "command=", "firstFrame=", "startFrame=", "endFrame=", "sourceFilePath=", "project=", "clipName=", "watchdir=", "tempdir=", "group=", "desk=", "reel=", "imageMode=", "layer=", "resultFilePath=", "logFilePath=", "outputDirPath=", "shot=", "slotLength=", "aspectRatio=", "user=", "currentDate="])
    except getopt.error, msg:
        print msg
        print "for help use --help"
        sys.exit( 1 )

    # process options
    try:
        if 0 == len( opts ):
            print __doc__
            sys.exit( 0 )
   
        for o, a in opts:
            if o in ("-h", "--help"):
                print __doc__
                sys.exit( 0 )

            if o in ("--command"):
                gCommand = str( a )  

            if o in ("--firstFrame"):
                try:
                    gFirstFrame = int( a ) 
                except:
                    raise StandardError( "start frame value is invalid" )                 

            if o in ("--startFrame"):
                try:
                    gStartFrame = int( a ) 
                except:
                    raise StandardError( "start frame value is invalid" )                 

            if o in ("--endFrame"):
                try:
                    gEndFrame = int( a )
                except:
                    raise StandardError( "end frame value is invalid" )

            if o in ("--sourceFilePath"):
                gSourceFilePath = str( a )  
    
            if o in ("--project"):
                gProject = str(a)

            if o in ("--clipName"):
                gClipName = str(a)

            if o in ("--watchdir"):
                gWatchdir = str(a)

            if o in ("--tempdir"):
                gTempdir = str(a)

            if o in ("--group"):
                gGroup = str(a)

            if o in ("--desk"):
                gDesk = str(a)
                    
            if o in ("--reel"):
                gReel = str(a)

            if o in ("--imageMode"):
                gImageMode = str(a)

            if o in ("--layer"):
                gLayer = str(a)

            if o in ("--resultFilePath"):
                gResultFilePath = str(a)

            if o in ("--logFilePath"):
                gLogFilePath = str(a)

            if o in ("--outputDirPath"):
                gOutputDirPath = str(a)

            if o in ("--shot"):
                gShot = str(a)

            if o in ("--slotLength"):
                gSlotLength = str(a)

            if o in ("--aspectRatio"):
                gAspectRatio = str(a)

            if o in ("--user"):
                gUser = str(a)

            if o in ("--currentDate"):
                gCurrentDate = str(a)
          
    except StandardError, msg:
        print >> sys.stdout, ( "ERROR: could not parse all parameters, see stderr for details" )
        print >> sys.stderr, ( "ERROR: %s\n" % msg )
        print >> sys.stdout, ""
        print >> sys.stderr, ""
        sys.exit( 1 ) 


    # --- RUN --- #
    try:
        # print parameters
        printParameters()

        # calculate frame range
        gStartFrame, gEndFrame = calculateFrameRange( gFirstFrame, gStartFrame, gEndFrame )

        # check path existance
        createDirIfNotExists( gTempdir )
        createDirIfNotExists( gWatchdir )
        createDirIfNotExists( gOutputDirPath )

        # setup file path 
        tempXMLFilePath = os.path.join( gTempdir, "%s.xml" % gClipName )
        XMLFilePath = os.path.join( gWatchdir, "%s.xml" % gClipName )

        # create XML file and put it in temp dir
        createXMLFile( tempXMLFilePath )

        # copy xml from tempdir to watchdir
        shutil.copyfile( tempXMLFilePath, XMLFilePath )  
        
        # check if scratch finish processing
        if not waitForScratch():
            #nuke.message( "Timeout." )
            sys.exit( 1 )

        #resultDict = readResultXML()
        #if resultDict['accept'] != 'true':
        #    sys.exit( 1 )


    except StandardError, msg:
        print >> sys.stdout, ( "\nERROR: %s" % msg )
        print >> sys.stderr, ( "\nERROR: %s" % msg )
        print >> sys.stdout, "" 
        print >> sys.stderr, ""
        sys.exit(1)

def printParameters():
    print "gFirstFrame     :  %s " % str( gFirstFrame       ) 
    print "gStartFrame     :  %s " % str( gStartFrame       )  
    print "gEndFrame       :  %s " % str( gEndFrame         )  
    print "gSourceFilePath :  %s " % str( gSourceFilePath   ) 
    print "gProject        :  %s " % str( gProject          )  
    print "gClipName       :  %s " % str( gClipName         )  
    print "gWatchdir       :  %s " % str( gWatchdir         )  
    print "gTempdir        :  %s " % str( gTempdir          )  
    print "gGroup          :  %s " % str( gGroup            )  
    print "gDesk           :  %s " % str( gDesk             )  
    print "gReel           :  %s " % str( gReel             )  
    print "gImageMode      :  %s " % str( gImageMode        )  
    print "gLayer          :  %s " % str( gLayer            )  
    print "gResultFilePath :  %s " % str( gResultFilePath   ) 
    print "gLogFilePath    :  %s " % str( gLogFilePath      )  
    print "gOutputDirPath  :  %s " % str( gOutputDirPath    )  
    print "gShot           :  %s " % str( gShot             )  
    print "gSlotLength     :  %s " % str( gSlotLength       )  
    print "gAspectRatio    :  %s " % str( gAspectRatio      )  
    print "gUser           :  %s " % str( gUser             )  
    print "gCurrentDate    :  %s " % str( gCurrentDate      )  

def calculateFrameRange( firstFrame, startFrame, endFrame ):
    print "calculating frame range..."
    # we need to use a frame range that offset from the first frame
    begin = startFrame - firstFrame
    end = endFrame - firstFrame
    return (begin, end)

def readResultXML():
    # TODO: read XML file and get the status 
    sch = ScratchContentHandler()
    parse( gResultFilePath, sch )
    return sch.returnDict

def getResultStatus( ):
    print "get result status..."
    file = open( gResultFilePath, 'r' )
    content = file.read()
    file.close()
    return content

def getLogStatus():
    print "get log status..."
    file = open( gLogFilePath, 'r' )
    content = file.read()
    file.close()
    return content


def waitForScratch( timeout = 120 ):
    print "waiting for scratch..."
    print "-timeout: %d" % timeout
    waitingTime = 0
    sleepingTime = 10
    print "waiting for result file..."
    while True:
        print "-waitingtime: %s" % waitingTime
        # if result/log file exists, break the loop
        if os.path.exists( gResultFilePath ):
            print getResultStatus()
            # delay for a few seconds to let Windows close the file
            time.sleep( 2 )
            print "removing %s" % gResultFilePath
            print os.remove( gResultFilePath )
            break
            #return True 
        if waitingTime > timeout :
            return False
        time.sleep( sleepingTime )    
        waitingTime += sleepingTime 

    waitingTime = 0
    print "waiting for log file..."
    while True:
        print "-waitingtime: %s" % waitingTime
        # if result/log file exists, break the loop
        if os.path.exists( gLogFilePath ):
            print getLogStatus()
            print "removing %s" % gLogFilePath
            print os.remove( gLogFilePath )
            return True 
        if waitingTime > timeout :
            return False
        time.sleep( sleepingTime )    
        waitingTime += sleepingTime

def isReadNode( node ):
    print "check node type..."
    return node.Class() == 'Read'

def createDirIfNotExists( dir ):
    os.umask(0002)
    print "check an existence of '%s'" % dir
    if not os.path.exists( dir ):
        print "make directory: %s" % dir
        os.makedirs( dir )

def createXMLFile( filename ) :
    print "creating xml file..."
    xmlstring = '''\
<?xml version="1.0"?>
<commands>
    <result file="%s"></result>
    <log file="%s"></log>
    <command name="%s">
        !-- source is where it grabs data -->
        <source>%s</source>
        <!-- path is output path -->
        <path>%s</path>
        <range>%s,%s</range>
        <length> %s </length>
        <project>%s</project>
        <group>%s</group>
        <desk>%s</desk>
        <reel>%s</reel>
        <image_mode>"%s"</image_mode>
        <layer>%s</layer>
        <clipname>%s</clipname>
        <note>"%s Sent By : %s on %s"</note>
    </command>
</commands>       
''' % ( gResultFilePath, gLogFilePath, gCommand, gSourceFilePath, gOutputDirPath, str(gStartFrame), str(gEndFrame), str(gSlotLength), gProject, gGroup, gDesk, gReel, gImageMode, str(gLayer), gClipName, gShot, gUser, gCurrentDate )
    file = open( filename, 'w' )
    file.write( xmlstring )
    file.close()
    pass


class ScratchContentHandler(ContentHandler):
   
    ## The constructor.
    # @param self The object pointer
    # @param xmlhandler XMLHandler instance 
    def __init__( self ):
        self.returnDict = {}
        self.loginfoTag = False
        pass

    ## Start Element [ContentHandler function]
    def startElement(self, name, attrs):
        # process xml   
        if name == 'command':    
            commandName = attrs.get( "name", None )            
            commandAccept = attrs.get( "accept", None )
            self.returnDict['command'] = commandName
            self.returnDict['accept'] = commandAccept
        
        return
        
    

if __name__ == "__main__":
    main()
