#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

def readFromWrite(nodes = ''):
  'Show a directors from a Read or a Write nodes'
  import os
  import nuke
  
  #if node nodes are give then look for selected nodes
  if nodes == '':
    nodes = nuke.selectedNodes()
  
  #Check to see if any nodes are selected
  if nodes == ():
    nuke.message('ERROR: No selected node')
    return
  
  #work around to select single node as an object
  node = nodes[0]
  _class = node.Class()
  if _class == "Write":
    
      knobs = ''
      
      file = node.knob('file').getValue()
      if file != '' and file is not None:
        knobs += ' file ' + file
      
      proxy = node.knob('proxy').getValue()
      if proxy != '' and proxy is not None:
        knobs += ' proxy ' + proxy
      
      first = nuke.toNode('root').knob('first_frame').getValue()
      knobs += ' first ' + str(first)
      
      last = nuke.toNode('root').knob('last_frame').getValue()
      knobs += ' last ' + str(last)

      xpos = node.knob('xpos').getValue()
      knobs += ' xpos ' + str(xpos)
      
      ypos = node.knob('ypos').getValue()
      ypos = int(ypos) + 40
      knobs += ' ypos ' + str(ypos)

      #Create directory
      ##FIX ME: make this platform indepened
      nuke.createNode('Read', knobs)  


  return

 
 
 
