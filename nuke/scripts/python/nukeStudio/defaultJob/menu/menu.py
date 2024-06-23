#   Copyright (c) Chris Bankoff.  All Rights Reserved
#   
#   Nuke Studio Job Menu
#
#

#short cut keys
# 'ctrl+s'        "^s"
# 'ctrl+shift+s'  "^+S"
# 'atl+shift+s'   "#+S"
# 'shitf+F1'      "+F1"

#######################################################
#    JOB MENU 
#######################################################


from nuke import menu

#tries to see get the studio name from the studio package
try:
  from nukeStudio import JOB_NAME
  jobName = JOB_NAME
except:
  jobName = Job


##Create Studio Menu in Nuke
menubar = menu('Nuke')
job_menu = menubar.addMenu(jobName)

#Job Gizmo-SubMenu
jobGizmoMenu = job_menu.addMenu("gizmos")
jobGizmoMenu.addCommand("Update", "nukeStudio.tools.buildGizmoMenu(jobGizmoMenu,nukeStudio.JOB_CUSTOM_PATH)")

#Add Menu Items
job_menu.addCommand("&HellowWorld", "nukeStudio.helloWorld()", "+F9")




