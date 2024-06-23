#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 


def shotNavigator():
  import nuke
  from nukeStudio.structure import shots
  
  t = nuke.ToolBar("Shot_Navigator")
  tm = t.addMenu("sub")
  
  shotList = shots()
  
  if shotList == None:
    return 
  
  for s in shotList:
    
    menuCommand = "nukeStudio.tools.simpleShotOpen(\'" + str(s) + "\')"
    
    t.addCommand(s, menuCommand, "")
    tm.addCommand(s, menuCommand, "")
  
  tm.addCommand('work.userName', "nuke.message('coming soon.')" "")
  
  #nuke.message('shot Navigator')
  return

def simpleShotOpen(shot):
    import nuke
    from nukeStudio.structure import currentStructure
    from nukeStudio.structure import fileStructure
    
    #Get the job from current structure
    job = currentStructure.get('JOB_DIR')
    
    #Create a temp structur to to ask for job paths
    tempStructure = fileStructure()
    tempStructure.setJob(job)
    tempStructure.setShot(shot)
    
    path = tempStructure.get('COMP_SCRIPTS_PATH')
    
    nkFile = nuke.getFilename('Open Shot', '*.nk', path)
    
    if nkFile != None :
      nuke.scriptOpen(nkFile)
    return




