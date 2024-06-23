# Run brave and smart djv_view on a Read or Write node. See the flipbook command
# for how we run djv_view on *any* node.
#
# put the script into:
# {NUKE_PATH}/nukescripts
#
# put the line:
# 'from djv_this import *'
# into your {NUKE_PATH}/nukescripts/__init__.py
#
# and put the line:
# m.addCommand("&Djv_view Selected", "nukescripts.flipbook(nukescripts.djv_this, nuke.selectedNode())", "#F")
# into your
# {NUKE_PATH}/plugins/menu.py
#

import platform
import sys
import os.path
import re
import thread
import nuke

djvViewPath= "/usr/local/djv/bin/djv_view"

def djv_this(node, start, end, incr, view):
  
  global djvViewPath
  filename = nuke.filename(node)
  if filename is None or filename == "":
    raise RuntimeError("Framecycler cannot be executed on '%s', expected to find a filename and there was none." % (node.fullName(),) )

  if not os.access(djvViewPath, os.X_OK):
    raise RuntimeError("check install: the script seeks this directory : /usr/local/djv/bin \nDjv_view cannot be executed (%s)." % (djvViewPath,) )


  fileName0=filename.split("%")[0]
  tag= filename.split(".")[-1]
  pad=filename.split('.')[-2][-2]
  first=str(start).zfill(int(pad))
  last=str(end).zfill(int(pad))

  padding=str(first) + "-" + str(last)
  
  FN = fileName0 + padding + "." + tag
    

  sequence_interval = str(start)+"-"+str(end)
  
 
  os.path.normpath(filename)
  args = []
  djvViewPath = os.path.normpath(djvViewPath)
  args.append( djvViewPath )
  args.append(FN)

  nuke.IrToken()
  os.spawnv(os.P_NOWAITO, djvViewPath, args)
