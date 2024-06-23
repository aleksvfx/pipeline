import nuke
import os
import sys
import shutil
import getpass
import time
from time import gmtime, strftime
from datetime import date
from xml.sax.handler import ContentHandler
from xml.sax import parse

class SendToScratch():
    def __init__( self, selectedNode, campaign_job ):
        # input from user : sourceFilePath, importOffset, project
        self.node           =   selectedNode 
        self.nodeName       =   selectedNode['name'].value()
        self.startFrame     =   selectedNode['first'].value()
        self.endFrame       =   selectedNode['last'].value()
        self.sourceFilePath =   selectedNode['file'].value() # need to translate to Windows format

        # fix value : group, desk, reel
        # need to be a unix path to the windows storage (F:\Scratch_Watch)
        self.watchdir       =   "/jobs/camptest1/camptest1_mocha/sequences/MOC/MOC001/nuke/scratch/watch"

        # unix path 
        self.tempdir        =   "/jobs/camptest1/camptest1_mocha/sequences/MOC/MOC001/nuke/scratch/temp" 

        # windows path
        self.outputDirPath  =   "\\\\frack\\jobs\\camptest1\\camptest1_mocha\\sequences\\MOC\\MOC001\\nuke\\scratch\\output" 
        self.project        =   campaign_job
        self.group          =   "Import"
        self.desk           =   date.today().strftime("%a_%m-%d-%y")  #"Thu_01-15-09" 
        self.reel           =   "last" 
        self.image_mode     =   "true" 
        self.slotLength     =   0
        self.layer          =   0
        self.logFilePath    =   "%s\\%s" % ( self.outputDirPath, "log.xml" )
        self.resultFilePath =   "%s\\%s" % ( self.outputDirPath, "result.xml" )
        self.shot           =   "DefaultImport"
        self.sender         =   getpass.getuser()
        self.currentDate    =   strftime("%a, %d %b %Y %H:%M:%S +0000", gmtime())
        
        # get output filename from source file e.g, /source/file/abc123.dpx ->  abc123
        self.clipName       =   ( ( os.path.basename( self.sourceFilePath ) ).split('.') )[0] 
        pass

    def run( self ):
        #nuke.message("Running Send To Scratch ...")

        # check if the selected node is a READ node 
        if not self.isReadNode( self.node ):
            pass
            #nuke.message( "Selected node is not a READ node." )
        
        # check path existance
        self.createDirIfNotExists( self.tempdir )
        self.createDirIfNotExists( self.watchdir )

        # setup file path 
        tempXMLFilePath = os.path.join( self.tempdir, "%s.xml" % self.clipName )
        XMLFilePath = os.path.join( self.watchdir, "%s.xml" % self.clipName )

        # create XML file and put it in temp dir
        self.createXMLFile( tempXMLFilePath )

        # copy xml from tempdir to watchdir
        shutil.copyfile( tempXMLFilePath, XMLFilePath )  
        
        # check if scratch finish processing
        if not self.waitForScratch():
            #nuke.message( "Timeout." )
            sys.exit( 1 )

        # check status and return a message 
        #nuke.message( self.getStatus() )
        pass

    def getStatus( self ):
        file = open( self.resultFilePath, 'r' )
        content = file.read()
        file.close()
        return content
        # TODO: read XML file and get the status 
        #sch = ScratchContentHandler()
        #parse( self.resultFilePath, sch )
        #return self.returnDict
        pass

    def waitForScratch( self, timeout = 120 ):
        waitingTime = 0
        sleepingTime = 10
        while True:
            # if result/log file exists, break the loop
            if os.path.exists( self.resultFilePath ) or os.path.exists( self.logFilePath ):
                return True 
            if waitingTime > timeout :
                return False
            time.sleep( sleepingTime )    
            waitingTime += sleepingTime 

    def isReadNode( self, node ):
        return node.Class() == 'Read'

    def createDirIfNotExists( self, dir ):
        if not os.path.exists( dir ):
            os.makedirs( dir )

    def createXMLFile( self, filename ) :
        xmlstring = '''\
<?xml version="1.0"?>
<commands>
    <result file="%s"></result>
    <log file="%s"></log>
    <command name="LOAD">
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
''' % ( self.resultFilePath, self.logFilePath, self.sourceFilePath, self.outputDirPath, str(self.startFrame), str(self.endFrame), str(self.slotLength), self.project, self.group, self.desk, self.reel, self.image_mode, str(self.layer), self.clipName, self.shot, self.sender, self.currentDate )
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
        
        elif name == 'loginfo':
            self.loginfoTag = True
        
        return
        
    def characters(self, content):
        if self.loginfoTag :
            self.returnDict['log'] = content
            self.loginfoTag = False
    
    ## End Element [ContentHandler function]
    def endElement(self, name):
        os.chdir("..")  # move up

