import nuke
import nukescripts
import sys 
import os
import subprocess
import re
import collect
import nuke_import_plates

this_filename = sys._getframe().f_code.co_filename
print >> sys.stderr, 'Loading Method facility '+this_filename


# Setup the import paths for method related stuff
kMethodToolsRoot = "METHOD_TOOLS_ROOT"
gToolsRoot       = "/tools/release"
gMethPyPath      = os.path.join( gToolsRoot, "lib/python" )
if kMethodToolsRoot in os.environ:
    gToolsRoot  = os.environ[kMethodToolsRoot]
    gMethPyPath = os.path.join( gToolsRoot, "lib/python" )
if (not gMethPyPath in sys.path):
    sys.path.append( gMethPyPath )


import launchdjv
if os.environ['SITE_'] == 'methodsm':
    import nukeStudio
elif os.environ['SITE_'] == 'methodny':
    import qubeSubmit
    import createDirectories
    import wtslate
import getpath

def setshotDirectory():
    # get environment variable
    # $SHOT_
    # $JOB_
    if 'CAMPAIGN_' in os.environ.keys():
        campaign = os.environ['CAMPAIGN_']
        if 'JOB_' in os.environ.keys():
            campaign_job = os.environ['JOB_']
            nuke.addFavoriteDir('JOB', getpath.getJobPath(campaign_job))

            if 'SEQUENCE_' in os.environ.keys():
                sequence = os.environ['SEQUENCE_']
                nuke.addFavoriteDir('SEQUENCE', getpath.getSequencePath(campaign_job, sequence))

                if 'SHOT_' in os.environ.keys():
                    shot = os.environ['SHOT_']
                    nukeDir = os.path.join(getpath.getShotPath(campaign_job, sequence, shot), 'nuke')
                    nuke.addFavoriteDir('SHOT', nukeDir)
                    #os.chdir(nukeDir)

    nuke.removeFavoriteDir('Nuke', False)


def sendToScratch():
    if nuke.selectedNode().Class() == 'Read':
        import methodSendToScratchDlg
        methodSendToScratchDlg.doIt(nuke.selectedNode())
    else:
        nuke.message("Please choose a READ node to send to scratch.")
    pass

def runSendToScratchStandalone():
    if nuke.selectedNode().Class() == 'Read':
        # construct a node dictionary
        selectedNode = nuke.selectedNode()
        node = {}
        node['name']        =   selectedNode['name'].value()
        node['file']        =   selectedNode['file'].value()
        node['start']       =   selectedNode['first'].value()
        node['first']       =   selectedNode['first'].value()
        node['end']         =   selectedNode['last'].value()
        node['pixelAspect'] =   selectedNode['format'].value().pixelAspect()

        retcode = subprocess.call('python %s --name "%s" --file "%s" --start "%s" --end "%s" --first "%s" --pixelAspect "%s" &' % (os.path.join(gToolsRoot, "lib/python/sendtoscratch/methodSendToScratchDlgStandalone.py"), str(node['name']), str(node['file']), str(node['start']), str(node['end']), str(node['first']), str(node['pixelAspect'])), shell=True)

        if retcode != 0:
            nuke.message("ERROR while running methodSendToScratchDlgStandalone.py")
    else:
        nuke.message("Please choose a READ node to send to scratch.")
    pass


def runMsend ():
    import msend.msendNuke
    msend.msendNuke.openMethodSendDialogNuke ()
    return


def runNukeSubmit():
    # get all enabled write node
    writeNodes = {}
    for node in nuke.allNodes("Write"):
        if node.knob('disable').getValue() == False:
            writeNodes[ node.name() ] = { 'ORDER' : str(node.knob("render_order").getValue()), 'OUTPUTDIR' :  str(nuke.filename(node)),
                                          'VIEWS' : node['views'].value().split() }
    # Save script
    nuke.scriptSave()

    # TODO: run command
    exePath = os.path.join( os.environ['JOB_TOOLS_ROOT_'], "bin", "NukeSubmit.py" )
    
    if not os.path.exists( exePath ):
        exePath = os.path.join(gToolsRoot, "lib/python/NukeSubmit/NukeSubmit.py")

    retcode = subprocess.call('python %s --version "%s" --file "%s" --start "%s" --end "%s" --writeNodes "%s" &' % ( exePath, nuke.NUKE_VERSION_STRING, nuke.knob("root.name"), nuke.knob('root.first_frame'), nuke.knob('root.last_frame'), str(writeNodes)), shell=True)

    if retcode != 0:
        nuke.message("ERROR while running NukeSubmit.py")
    pass

def openNukeScript():
    filepath = None
    if ('CAMPAIGN_' in os.environ.keys()) and ('JOB_' in os.environ.keys()) and ('SEQUENCE_' in os.environ.keys()) and ('SHOT_' in os.environ.keys()):
        shotPath = getpath.getShotPath(os.environ['JOB_'], os.environ['SEQUENCE_'], os.environ['SHOT_'])
        filepath = nuke.getFilename("Open Nuke Script", "*.nk", default = os.path.join(shotPath, 'nuke/'))
    else:
        filepath = nuke.getFilename("Open Nuke Script", "*.nk")
    if filepath != None:
        # Calling nuke.load () imports the script directly
        # into the current nuke session. Instead, we want to open
        # a new nuke window with the selected script
        # nuke.load(filepath)
        nuke.scriptOpen (filepath)
    pass

def addLightEffects():
    iconDir = os.path.join(os.environ['METHOD_TOOLS_ROOT'],'nuke/scripts/python/nukeStudio/menu/icons')
    m = toolbar.addMenu('Light Effects', os.path.join(iconDir,'LightEffects.png'))
    m.addCommand('Radiosity',"nuke.createNode('radiosity')", icon='Blur.png')
    m.addCommand('Split Radiosity',"nuke.createNode('SplitRadiosity')", icon=os.path.join(iconDir,'SplitRadiosity.png'))
    m.addCommand('RGB Blur',"nuke.createNode('RGBBlur')", icon='Spark.png')
    m.addCommand('Croma Aberation',"nuke.createNode('ChromAbb')", icon=os.path.join(iconDir,'ChromAbb.png'))
    m.addCommand('Lens Aberation',"nuke.createNode('Lens_Aberration')", icon=os.path.join(iconDir,'ChromAbb.png'))
    m.addCommand('Film Effect',"nuke.createNode('FilmEffect')", icon='Spark.png')
    m.addCommand('Film Curve',"nuke.createNode('FilmCurve')", icon='Spark.png')
    m.addCommand('Classic Vignette',"nuke.createNode('vignette')", icon='Spark.png')
    m.addCommand('Simple Glare',"nuke.createNode('glare_simple')", icon='Spark.png')
    m.addCommand('Light Wrap',"nuke.createNode('LightWrap')", icon='Spark.png')
    m.addCommand('Plate wrap',"nuke.createNode('plateWrap')", icon='Spark.png')
    m.addCommand('Plate Flash',"nuke.createNode('plateFlash')", icon='Spark.png')

def nydefaults():
    nuke.addFormat('1280 720 HD720')
    nuke.addFavoriteDir('Jobs', '/jobs')
    nuke.addFavoriteDir('Library', '/method/library')
    nuke.knobDefault('Bezier.linear', 'on')
#    nuke.knobDefault('Read.premultiplied', 'on')
#    nuke.knobDefault('Write.channels', 'rgba')
#    nuke.knobDefault('Write.premultiplied', 'on')
#    nuke.knobDefault('Write.file', '[file rootname [file tail [value root.name]]].%04d.jpg')

def setDefaultFont():
    default_font = "/tools/apps/share/fonts/Vera.ttf"
    nuke.knobDefault('Text.font', default_font)

def customizeMainMenu():
    menuBar = nuke.menu('Nuke')
    methodMenu = menuBar.addMenu("&Method")
    renderMenu = menubar.addMenu("&Render")
    methodMenu.addCommand('Collect Footage','collect.collectFootage()')
    methodMenu.addCommand("Send to Scratch (Standalone)", "runSendToScratchStandalone()", "F9")
    methodMenu.addCommand("Open Nuke script", "openNukeScript()", "^o")
    if os.environ['SITE_'] == 'methodsm':
        methodMenu.addCommand("-", "", "")
        methodMenu.addCommand("Submit to Farm", "runNukeSubmit()", "F6")

    elif os.environ['SITE_'] == 'methodny':
        methodMenu.addCommand("Create Render Directories", "createDirectories.write_mkdir()", "F8")
        methodMenu.addCommand("Submit to Qube", "qubeSubmit.submitNuke()", "F6")

    # facility default convention for plate names

    root_menu = menuBar
    # This has to be done out in show specific menu.py
    # copy and paste
    #methodMenu = root_menu.findItem ("Method")
    #if (methodMenu == None):
    #    methodMenu = menubar.addMenu( "Method")
    #import_menu = methodMenu.findItem ("Import")
    #if (import_menu == None):
    #    import_menu = methodMenu.addMenu( "Import")
    # # alt-p import plate for current shot
    # import_menu.addCommand("Import Plate", 
    #     "nuke_import_plates.nuke_importPlates ()", "#p")
    ## alt-shift-p ask user to shot name and import plates for that
    #import_menu.addCommand("Import Plate - other",
    #    "nuke_import_plates.nuke_importPlatesAskUser ()", "#+p")
    pass

def addMethodToolbar():
    iconDir = os.path.join(os.environ['METHOD_TOOLS_ROOT'], "nuke/resources/icon")
    toolbar = nuke.toolbar("Nodes")
    if os.environ['SITE_'] == 'methodsm':
        # disable default flipbook menu
        nuke.menu("Nuke").findItem("Render").findItem("Flipbook Selected").setEnabled( False )

        sys.path.append( os.path.join(os.environ['METHOD_TOOLS_ROOT'], "lib", "python" ) )
        toolbar.addCommand("Flipbook", "import MethodPlatform.Nuke.Flipbook;MethodPlatform.Nuke.Flipbook.flipbook( MethodPlatform.Nuke.Flipbook.framecycler_this, nuke.selectedNode()) ", "#F", icon= os.path.join(iconDir, "flipbook.png"))
        # toolbar.addCommand("Flipbook", "nukescripts.flipbook(nukescripts.framecycler_this, nuke.selectedNode()) ", icon= os.path.join(iconDir, "flipbook.png"))
#        toolbar.addCommand("Flipbook (djv_view)", "launchdjv.run(nuke.selectedNode())" , icon= os.path.join(iconDir, "flipbook.png"))
        toolbar.addCommand("Send To Scratch", "runSendToScratchStandalone()", icon = os.path.join(iconDir, "sendtoscratch.png"))
    elif os.environ['SITE_'] == 'methodny':
        toolbar.addMenu("Method", icon=os.path.join(iconDir,"method_nuke.png"))
        toolbar.addCommand("Method/DropShadow", "nuke.createNode('DropShadow')")
        toolbar.addCommand("Method/SlateAndProxy", "nuke.createNode('SlateAndProxy')")
        toolbar.addCommand("Method/MethodBug", "nuke.createNode('methodBug')")
        toolbar.addCommand("Method/Write + Latest", "wtslate.wtslate()")
        # add some hot keys
        nuke.menu('Nodes').addMenu('Channel').addCommand('Shuffle','nuke.createNode(\'Shuffle\')','l')
        addLightEffects()
    pass

def addPlugins():
    """ Add the facility level commpiled plugins """
    # THe toolbar "Nodes" is right click in node graph AND the toolbar at the left edge of Nuke
    toolbar = nuke.toolbar("Nodes")
    transformMenu = toolbar.findItem ("Transform")
    transformMenu.addCommand("GridWarp2", "nuke.createNode(\"GridWarp\")", icon="GridWarp.png")
    toolbar = nuke.toolbar("Nodes")
    pass

if __name__ == "__main__":
    setshotDirectory()
    addPlugins()
    customizeMainMenu()
    addMethodToolbar()

    if (len (sys.argv) == 0):
        sys.argv = [ "nuke" ]

    if os.environ['SITE_'] == 'methodsm':
        setDefaultFont()
    elif os.environ['SITE_'] == 'methodny':
        nydefaults()
    else:
        nuke.tprint(this_filename+': Unknown SITE_')

# vim:ts=4:sw=4:expandtab

