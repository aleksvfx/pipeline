# collect.py
#
# collect all the footage of a nuke script and consolidate to one directory.
# Ask Pink about it.
#

# TODO : ensure no read nodes are in error state

import os, sys, shutil, glob, errno, re, platform
import nuke
#import nukescripts
import mpath

this_filename = sys._getframe().f_code.co_filename
print('Loading '+this_filename)

platform = platform.system()

def destinationDir(hint):
  response = str()
  while True:
    if os.path.exists(response) == False:
      response = nuke.getInput("Directory to store collected images", hint)
      if response == None:
        break
      if os.path.exists(response) == False:
        if nuke.ask("No such directory. Create it?") == True:
          try:
            os.makedirs(response, 0775)
          except OSError, e:
            nuke.message(os.strerror(e.errno))
    else:
      break
  return response
#  panel = nuke.Panel("Collect images")
#  panel.addSingleLineInput('Collect images to', scriptFullName)
#  panel.show()
#  cpath = panel.value("Collect images to")
#  print cpath

def collectFootage():
  nkScriptFullName = nuke.toNode("root").name()
  nkScriptName = nkScriptFullName.split('/').pop().split('.')[0]

  if nkScriptName == 'Root':
    raise RuntimeError(this_filename+"\ncollectFootage() : Please save Nuke script before using.")

  shotDir = mpath.shotDirForFilePath(nkScriptFullName)
  print '# Shot Dir.        = '+shotDir
  print '# Nuke Script Name = '+nkScriptName

  dest = destinationDir(shotDir+'/images/renders/'+nkScriptName+'_collected')
  if dest == None:
    return

  allnodes = nuke.allNodes(group=nuke.root())

  # Ask user if we want to include images that are in the plates dir.
  include_plates = False
  for node in allnodes:
    klass = node.Class()
    if klass == "Read":
      name = nuke.filename(node)
      if '/plates/' in name:
        include_plates = nuke.ask('Include footage from the plates/ directories?')
        break

  # on unix we don't have to copy, we can use hardlinks, so warn lusers
  if platform != 'Linux':
    nuke.message('Copying footage on non-unix systems may take awhile.')

  for node in allnodes:
    klass = node.Class()
    if klass == "Read":
      name = nuke.filename(node)
      base = name.split('/').pop()
      nameglob = re.sub('\.%.*\.','.*.',name)
#      print '# Name      = '+name
#      print '# Name Glob = '+nameglob
      if '/plates/' in name and include_plates == False:
        continue
      for file in glob.glob(nameglob):
        print '# '+file+' -> '+dest
        if platform == 'Linux':
          hardlink = dest+'/'+os.path.basename(file)
          if not os.path.exists(hardlink):
            os.link(file, hardlink)
        else:
          shutil.copy2(file, dest)

      newfilename = dest+'/'+base
      node['file'].setValue(newfilename)

  nuke.message('Footage gathered and Read paths altered. Please Save-As.')
  print '# Destination = '+dest

# vim:ts=2:sw=2:expandtab

