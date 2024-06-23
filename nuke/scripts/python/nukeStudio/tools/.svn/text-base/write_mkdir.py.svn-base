#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 
import os
import os.path
import re
from os import mkdir, system
from os import makedirs
import nuke

viewRe = re.compile("(%V)")
def write_mkdir(nodes = ''):
  'Make directors from write nodes'
  
  #if node nodes are give then look for selected nodes
  if nodes == '':
    nodes = nuke.selectedNodes()
  
  #Check to see if any nodes are selected
  if nodes == ():
    nuke.message('ERROR: No nodes are selected')
    return
   
  for i in nodes:
    _class = i.Class()
    if _class == "Write":
      
      # use this to get only the filename
      paths = [nuke.filename(i)]
      
      #if empty continue
      if paths is None:
        continue
      
      #Check to see if there are stereo views
      if viewRe.search(paths[0]):
        template = paths[0]
        paths = []
        for view in i['views'].value().split():
          paths.append(re.sub(viewRe,view,template))
          
      for path in paths:
          #Get Directory Structure with out file name
          dirPath = os.path.dirname(path)
          
          if os.path.isdir (dirPath):
            print "Your directory %s already exists" % (dirPath)
            return
    
          if os.path.isfile (dirPath):
            # The folder exists as a file
            msg = "The directory:\n%s\nexists as a file. Delete it and create folder?" % (dirPath)
            delete_existing_file = nuke.ask (msg)
            if (delete_existing_file):
                os.unlink (dirPath)
            else:
                return
            
          #Create directory
          try:
            makedirs (dirPath)
            # Some users seem to have a umask that defaults to
            # no-write-perms for group. Farm jobs run as qubeproxy
            # in the production group, so we chmod manually.
            cmd = "chmod 775 %s" % (dirPath)
            print "Created %s and updated group write permissions" % (dirPath)
            os.system (cmd)
          except:
            if nuke.ask('Create Path: '+ dirPath):
              makedirs(dirPath)
            else:
              return

  return

