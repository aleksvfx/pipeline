#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
#   FIX ME:  Make this platform independent

def show_dir(nodes = ''):
  'Show a directors from a Read or a Write nodes'
  import os
  import nuke
  import platform
  
  #if node nodes are give then look for selected nodes
  if nodes == '':
    nodes = nuke.selectedNodes()
  
  #Check to see if any nodes are selected
  if nodes == ():
    nuke.message('ERROR: No selected node')
    return
  
  for i in nodes:
    _class = i.Class()
    if _class == "Write" or _class == "Read" :
      
      # use this to get only the filename
      path = nuke.filename(i)
      
      #if empty continue
      if path is None:
        continue
      
      #Get Directory Structure with out file name
      dirPath = os.path.dirname(path)

      #Create directory
      ##FIX ME: make this platform indepened
      if platform.system() == 'Linux':
      
        if os.path.exists('/usr/bin/nautilus'):
          os.popen2('/usr/bin/nautilus ' + dirPath)
          
        if os.path.exists('/usr/bin/konqueror'):
          os.popen2('/usr/bin/konqueror ' + dirPath)
          
      elif platform.system() == 'Darwin':
        os.popen2('open ' + dirPath)

  return

