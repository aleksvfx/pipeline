#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 



def buildGizmoMenu(menuObject, gizmoLocation):

  import os.path
  import nuke
  import re

  #Empy Gizmo List
  gizmoList = []
  #gizmoSubMenus []

  #
  #plugins = nuke.plugins(nuke.ALL | nuke.NODIR, "*."+nuke.PLUGIN_EXT, "*.ofx.bundle", "*.gizmo")

  #list of all the loaded gizmos
  loadedGizmos = nuke.plugins(nuke.ALL, "*.gizmo")

  regGizmo = re.compile("^"+ gizmoLocation +".*/(\w*).gizmo$")

  for i in loadedGizmos:
    #(root, ext) = os.path.splitext(i)
    #(root, ext) = os.path.splitext(root)
    # Sometimes we get files like ._Glare.gizmo, etc due to Mac OS X
    # or editors and that returns an empty string for root.
    
    #if empty 
    #if root is None or len(root) == 0:
      #continue

    regMatch = regGizmo.search(i)

    if regMatch == None:
      continue
    
    gizmoName = regMatch.group(1)
    #print gizmoName
    print i
    
    gizmoList.append(gizmoName)
    
    #if not root[0] == '@':
      #gizmoSubMenus.append(root)
    #foo

  #n = nuke.menu("Nodes").addMenu("Other/"+menuname)
  
  gizmoList.sort()
  
  if len(gizmoList) > 10:
    subMenuOpt = True
  else:
    subMenuOpt = False
  
  for i in gizmoList:
    if subMenuOpt:
      subMenu = i.upper()[0]
      menuItem = menuObject.addMenu(subMenu)
      gizmoMenu = menuItem.addCommand(i, "nuke.createNode('"+i+"')")
    else:
      gizmoMenu = menuObject.addCommand(i, "nuke.createNode('"+i+"')")

  return
