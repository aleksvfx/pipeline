import os
import re

import nuke

import nodeOps
import quick3d
import panAndTile
import autoBackdrop
import fixPaths
import customNode
import papiTools
import nodeTools


UsersToolbar = nuke.toolbar('Nodes')

myMenu = UsersToolbar.addMenu('xXx', 'POINTCACHE3D.png')

myMenu.addCommand("FloatView", "nuke.createNode('FloatView')", 'alt+f', icon="FloatView.png")
myMenu.addCommand("FloatView OTHER/FloatView", "nuke.createNode('FloatView')", 'alt+f', icon="FloatView.png")
myMenu.addCommand("FloatView OTHER/ViewFloat", "nuke.createNode('ViewFloat')", icon="FloatView.png")
myMenu.addCommand("FloatView OTHER/ViewZ", "nuke.createNode('ViewZ')", icon="FloatView.png")
myMenu.addCommand("FloatView OTHER/ZDepthView", "nuke.createNode('ZDepthView')", icon="FloatView.png")

myMenu.addCommand("SampleImageMinMax", "nuke.createNode('SampleImageMinMax')", icon="SampleImageMinMax.png")

myMenu.addCommand("GrayScaleImage2NormalMap", "nuke.createNode('GrayScaleImage2NormalMap')", 'alt+n' , icon="GrayScaleImage2NormalMap.png")
myMenu.addCommand("GrayScaleImage2NormalMap3D [SLOW 3D]", "nuke.createNode('GrayScaleImage2NormalMap3D')", icon="GrayScaleImage2NormalMap3D.png")


myMenu.addCommand("VecDepth2Position", "nuke.createNode('VecDepth2Position')", 'alt+v' , icon="VecDepth2Position.png")
myMenu.addCommand("VecDepth2PositionImage", "nuke.createNode('VecDepth2PositionImage')", icon="VecDepth2Position.png")


myMenu.addCommand("VecPosition2Normal", "nuke.createNode('VecPosition2Normal')", icon="VecPosition2Normal.png")



myMenu.addCommand("POINTCACHE3D", "nuke.createNode('POINTCACHE3D')", '+3', icon="POINTCACHE3D.png")




myMenu.addCommand("ReLight", "nuke.createNode('ReLight')", 'alt+r', icon="ReLight.png")




myMenu.addCommand("iMultMerge [SHAKE (A*B)]", "nuke.tcl('Merge')", 'ctrl+m', icon="iMultMerge.png")



myMenu.addCommand("UVmap [STMap]", "nuke.createNode('STMap')", icon="UVmap.png")









UsersToolbar.addCommand("Undo", "nuke.undo()", icon="NUKE_Undo.png")
UsersToolbar.addCommand("Redo", "nuke.redo()", icon="NUKE_Redo.png")
UsersToolbar.addCommand("VIEWER OnOff", "nodeOps.toggleViewerPipes()", 'alt+t', icon="ViewerOnOff.png")
UsersToolbar.addCommand("FullScreen", "nuke.toggleFullscreen()", 'Alt+S', icon="NUKE_FullSize.png")






STARTFORMAT = 'PAL'
nuke.knobDefault('Root.format', STARTFORMAT)
STARTFPS = '25'
nuke.knobDefault('Root.fps', STARTFPS)







###############################################################
### init.py - Example
###############################################################
#
# import os
#
# nuke.pluginAddPath('./gizmos')
# nuke.pluginAddPath('./python')
# nuke.pluginAddPath('./tcl')
# nuke.pluginAddPath('./icons')

### curShow = os.environ['SHOW']
### showPath = os.path.join( '/projects', curShow, 'nuke')
### if os.path.isdir( showPath ):
    ### nuke.pluginAddPath( showPath )


