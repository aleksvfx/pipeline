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

myMenu.addCommand("POINTCACHE3D", "nuke.createNode('POINTCACHE3D')", '+3', icon="POINTCACHE3D.png")