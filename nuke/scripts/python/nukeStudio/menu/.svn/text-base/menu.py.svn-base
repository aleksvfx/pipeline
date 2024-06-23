#   Copyright (c) Chris Bankoff.  All Rights Reserved
#   
#   Nuke Studio Custom Menu

#short cut keys
# 'ctrl+s'        "^s"
# 'ctrl+shift+s'  "^+S"
# 'atl+shift+s'   "#+S"
# 'shitf+F1'      "+F1"

#Add Menu System
nuke.pluginAddPath("./icons")

menubar = nuke.menu('Nuke')

#######################################################
#Edit Menu
#######################################################

m = menubar.addMenu("&Edit")
n = m.addMenu("&Node")
n.addCommand("-", "", "")
n.addCommand("Set BBox to A", "nukeStudio.tools.setBBox('A')", "#a")
n.addCommand("Set BBox to B", "nukeStudio.tools.setBBox('B')", "#b")
n.addCommand("Set BBox to union", "nukeStudio.tools.setBBox('union')", "#m")
n.addCommand("-", "", "")
n.addCommand("Read from Write", "nukeStudio.tools.readFromWrite()", "#r")

#######################################################
#Render Menu
#######################################################

#Create Studio Menu in Nuke
m = menubar.addMenu("&Render")
m.addCommand("-", "", "")
m.addCommand("Create Directories", "nukeStudio.tools.write_mkdir()", "F8")
m.addCommand("Show Directory", "nukeStudio.tools.show_dir()", "+F8")


#######################################################
#Help Menu
#######################################################

m = menubar.addMenu("&Help")
m.addCommand("-", "", "")
m.addCommand("Python Documentation", "webbrowser.open_new('http://www.python.org/doc/')", "F11")
m.addCommand("Python Search", "nukeStudio.tools.pythonDocSearch()", "+F11")
m.addCommand("-", "", "")


#######################################################
# Studio
#######################################################

#tries to see get the studio name from the studio package
try:
  from nukeStudio import STUDIO_NAME
  studioName = STUDIO_NAME
except:
  studioName = StudioMenu


##Create Studio Menu in Nuke
#menubar = nuke.menu('Nuke')

# - Boon edited -- 09/30/09
# -- Find "Tools" menu item and add submenu to it
#studio_menu = menubar.addMenu(studioName)
studio_menu = menubar.findItem("Method")
#studio_menu.addCommand("-", "", "")

#Add Menu Items
#studio_menu.addCommand("HellowWorld", "nuke.message('HelloWorld:properties_menu')", "")
#studio_menu.addCommand("Submit to Qube", "nukeStudio.qube.submitNuke()", "F6")
#studio_menu.addCommand("-", "", "")
#FIX FONT PROBLEMS: Path fix
studio_menu.addCommand("Burn-in Frame Number", "nuke.createNode('FrameNumber', 'font /S2/3d/farm/nuke/studio_custom/font/Vera.ttf')", "")
studio_menu.addCommand("Studio Slate", "nuke.createNode('studioSlate', 'font /S2/3d/farm/nuke/studio_custom/font/Vera.ttf')", "")
studio_menu.addCommand("-", "", "")

#Studio Gizmo-SubMenu
studioGizmoMenu = studio_menu.addMenu("gizmos")
studioGizmoMenu.addCommand("Update", "nukeStudio.tools.buildGizmoMenu(studioGizmoMenu,nukeStudio.NUKESTUDIO_PACKAGE_LOCATION)")

#studio_menu.addCommand("Firefox", "os.system('/usr/bin/firefox')", "")
#studio_menu.addCommand("Google", "webbrowser.open_new(\'www.google.com\')", "")



#######################################################
# TD Menu System
#######################################################


try:
  from nukeStudio import TD
  tdOpt = TD
except:
  tdOpt = 'False'

if tdOpt == 'True':
  from nukeStudio.menu import tdMenu


#######################################################
#JOB Menu
#######################################################

#tries to see get the studio name from the studio package
try:
  from nukeStudio import JOB_NAME
  jobName = JOB_NAME
except:
  jobName = Job


##Create Studio Menu in Nuke
#menubar = nuke.menu('Nuke')
# TODO
# job_menu = menubar.addMenu(jobName)

#Add Menu Items
#job_menu.addCommand("&HellowWorld", "nukeStudio.helloWorld()", "+F9")
  
#######################################################
#User Menu
#######################################################


#tries to see get the studio name from the studio package
try:
  from nukeStudio import USER_NAME
  userName = USER_NAME
except:
  userName = DefaultUser


##Create Studio Menu in Nuke
#menubar = nuke.menu('Nuke')
# TODO
# user_menu = menubar.addMenu(userName)

# u.addCommand("-", "", "")
# user_menu.addCommand("Modify User Settings", "nukeStudio.userSettings.modifyPrompt()", "")

# un = user_menu.addMenu("NetworkUsers")
# un.addCommand("Message", "nuke.message('Comming Soon....')", "")


####################################################################################
#     NODE MENU
####################################################################################

##Create Studio Menu in Nuke
toolbar = nuke.menu('Nodes')


#######################################################
#Image
#######################################################

m = toolbar.addMenu("Image", "ToolbarImage.png")


#######################################################
#Draw
#######################################################

m = toolbar.addMenu("Draw", "ToolbarDraw.png")
#m.addCommand("Text", "nukescripts.create_text()", "+T", icon="Text.png")
m.addCommand("Simple Grain Scan", "nuke.createNode('Simple_GrainScan', 'file /S2/3d/bank/DATA/grain_scans/ToyotaCorolla_Grain_Clean_Dustbust/Grain_Clean_Dustbust.%04d.cin')", "", icon="Spark.png")


#######################################################
#Time
#######################################################

m = toolbar.addMenu("Time", "ToolbarTime.png")


#######################################################
#Channel
#######################################################

m = toolbar.addMenu("Channel", "ToolbarChannel.png")
#n = m.addMenu("Shuffle From", "ToolbarChannel.png")
m.addCommand("Keep", "nuke.createNode('Remove', 'operation keep channels rgba')", icon="Keep.png")
m.addCommand("FloodRed", "nuke.createNode('Shuffle', 'green red blue red alpha red name FloodRed tile_color 0x7e0b0bff')", icon="FloodRed.png")
m.addCommand("FloodGreen", "nuke.createNode('Shuffle', 'red green blue green alpha green name FloodGreen tile_color 0x356b00ff')", icon="FloodGreen.png")
m.addCommand("FloodBlue", "nuke.createNode('Shuffle', 'red blue green blue alpha blue name FloodBlue tile_color 0x171789ff')", icon="FloodBlue.png")
m.addCommand("FloodApha", "nuke.createNode('Shuffle', 'red alpha green alpha blue alpha name FloodApha tile_color 0xffffffff')", icon="FloodAlpha.png")
#FIX ME: Create FloodLuma Gizmo
m.addCommand("FloodLuma", "nuke.createNode('FloodLuma')", icon="FloodLuma.png")
m.addCommand("Shuffle From", "nuke.createNode('Shuffle', 'in2 rgba red red2 green green2 blue blue2 label \[knob\ this.in2\]')", icon="Shuffle.png" )


#######################################################
#Color
#######################################################

m = toolbar.addMenu("Color", "ToolbarColor.png")

#######################################################
#Filter
#######################################################

m = toolbar.addMenu("Filter", "ToolbarFilter.png")

#######################################################
#Light Effets
#######################################################

m = toolbar.addMenu("Light Effects", "Toolbar3DLights.png")
m.addCommand("Radiosity", "nuke.createNode('radiosity')", icon="Blur.png" )
m.addCommand("Split Radiosity", "nuke.createNode('SplitRadiosity')", icon="SplitRadiosity.png" )
m.addCommand("RGB Blur", "nuke.createNode('RGBBlur')", icon="Spark.png" )
#m.addCommand("Croma Aberation", "nuke.createNode('ChromAbb')", icon="ChromAbb.png" )
m.addCommand("Lens Aberation", "nuke.createNode('Lens_Aberration')", icon="ChromAbb.png" )
m.addCommand("Film Effect", "nuke.createNode('FilmEffect')", icon="Spark.png" )
m.addCommand("Film Curve", "nuke.createNode('FilmCurve')", icon="Spark.png" )
m.addCommand("Classic Vignette", "nuke.createNode('vignette')", icon='Spark.png' )
m.addCommand("Simple Glare", "nuke.createNode('glare_simple')",  icon='Spark.png')
m.addCommand("Light Wrap", "nuke.createNode('LightWrap')",  icon='Spark.png')
m.addCommand("Plate wrap", "nuke.createNode('plateWrap')",  icon='Spark.png')
m.addCommand("Plate Flash", "nuke.createNode('plateFlash')",  icon='Spark.png')

#######################################################
#Keyer
#######################################################

m = toolbar.addMenu("Keyer", "ToolbarKeyer.png")


#######################################################
#Merge
#######################################################

m = toolbar.addMenu("Merge", "ToolbarMerge.png")
#m.addCommand("Merge", "nuke.createNode(\"Merge\")", "m", icon="Merge.png")
#m.addCommand("@;&MergeBranch", "nuke.createNode(\"Merge2\")", "+m")
#m.addCommand("@;&Mult-Merge", "nuke.createNode(\"Merge2\")", "+m")
n = m.addMenu("Merges", "Merge.png")


#######################################################
#Transform
#######################################################

m = toolbar.addMenu("Transform", "ToolbarTransform.png")


#######################################################
#3D
#######################################################

m = toolbar.addMenu("3D", "Toolbar3D.png")
m.addCommand("Chan Import", "nukeStudio.tools.chan_data.file_import()", "" , icon="Read.png")

#######################################################
#Views
#######################################################

m = toolbar.addMenu("Views", "ToolbarViews.png")


#######################################################
#Other
#######################################################

m = toolbar.addMenu("Other", "ToolbarOther.png")



#######################################################
# Folder in file brosers
#######################################################

#from nuke import tcl as addFolder
#import nuke

#nuke.tcl('add_favorite_dir jobName jobName icon_name toolTip /S2/3d/prod/testxxM000/')
    #This function adds an item to the file chooser's favorite directory list.
    #The name argument is the favourite list entry label ('Home', 'Desktop', etc.); directory is the filesystem path; type is optional and can be a bitwise OR combination of any of the constants nuke.IMAGE, nuke.SCRIPT, nuke.FONT, nuke.GEO, and nuke.PYTHON; tooltip is optional and is a short text explanatory description that appears when the pointer hovers over the favorite item; and key is an optional argument to add a shortcut key.
    #The path name can contain environment variables which will be expanded when the user clicks the favourite's button.


from nukeStudio.structure import currentStructure

#nuke.addFavoriteDir(name, directory, type, icon, tooltip, key)
# nuke.addFavoriteDir(STUDIO_NAME + ' Jobs', currentStructure.get('ROOT_JOB') , nuke.SCRIPT, 'drive', 'Jobs Directory', '')
# nuke.addFavoriteDir(STUDIO_NAME + ' Renders', currentStructure.get('ROOT_FRAMES') , nuke.IMAGE, 'drive', 'Renders Directory', '')

# nuke.addFavoriteDir(JOB_NAME , currentStructure.get('JOB_SHOTS_PATH') , nuke.SCRIPT, 'drive', 'Active Job Directory', '')
# nuke.addFavoriteDir(JOB_NAME + ' Renders', currentStructure.get('RENDERS_SHOT_PATH') , nuke.IMAGE, 'drive', 'Active Job Directory', '')


# nuke.addFavoriteDir(USER_NAME, currentStructure.get('USER_PATH') , nuke.IMAGE+nuke.SCRIPT, 'drive', 'Personal User Directory', '')
# nuke.addFavoriteDir('Network Fonts', currentStructure.get('NETWORK_FONTS') , nuke.FONT, 'drive', 'Personal User Directory', '')




