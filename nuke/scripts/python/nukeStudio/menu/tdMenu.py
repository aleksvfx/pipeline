#   Copyright (c) Chris Bankoff.  All Rights Reserved
#   
#   Nuke Studio Custom Menu
#
#

#short cut keys
# 'ctrl+s'        "^s"
# 'ctrl+shift+s'  "^+S"
# 'atl+shift+s'   "#+S"
# 'shitf+F1'      "+F1"

#######################################################
#TD Menu
#######################################################

from nuke import menu

##Create Studio Menu in Nuke
menubar = menu('Nuke')
td_menu = menubar.addMenu('TD')

#Add Menu Items
td_menu.addCommand("Load Development Environment", "Dev.modifyPrompt()", "")
td_menu.addCommand("Display Custom Info", "nuke.message(studioCustomInfo)", "")
td_menu.addCommand("-", "", "")
tn = td_menu.addMenu("Module Package")
td_menu.addCommand("-", "", "")
tn.addCommand("Modify Package Info", "\
pk = nukeStudio.managePackage()\n\
pk.updatePackageInfo()\n\
", "")
tn.addCommand("-", "", "")
tn.addCommand("Save Build", "\
pk = nukeStudio.managePackage()\n\
pk.archiveBuild()\n\
", "")
tn.addCommand("Save Version", "\
pk = nukeStudio.managePackage()\n\
pk.archiveVersion()\n\
", "")
tn.addCommand("Save Release", "\
pk = nukeStudio.managePackage()\n\
pk.archiveRelease()\n\
", "")
tn.addCommand("-", "", "")
tn.addCommand("Distribute Version", "\
pk = nukeStudio.managePackage()\n\
pk.distributeVersion()\n\
", "")
td_menu.addCommand("-", "", "")
tn = td_menu.addMenu("Vray")
td_menu.addCommand("-", "", "")
tn.addCommand("vrimg2exr", "nukeStudio.tools.vrimg2exr()", "" )
#n.addCommand("&HellowWorld", "nukeStudio.helloWorld()", "+F9")
td_menu.addCommand("Display Environment Variables", "nukeStudio.tools.disEnvVars()", "")
td_menu.addCommand("Modify User Local Settings", "nukeStudio.userSettings.modifyPrompt()", "")
td_menu.addCommand("-", "", "")

