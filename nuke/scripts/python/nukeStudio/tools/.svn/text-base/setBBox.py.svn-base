#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 



#########################################################
#    setBBox 
#########################################################
 
  
def setBBox(knob = 'union', nodes =''):
  'Changes the bbox settings for nodes need to specify A, B, or union'
  import nuke
  import re

  knobOpts = ['union', 'B', 'A' ]
   
  if knobOpts.count(knob) == 0:
    return 'ERROR: valid option ' + str(knobOpts)
    #FIX ME: ADD REAL raise ERROR 

  #if node nodes are give then look for selected nodes
  if nodes == '':
    nodes = nuke.selectedNodes()
  
  #Check to see if any nodes are selected
  if nodes == ():
    nuke.message('ERROR: No nodes are selected')
    return
   
  knobSetValue = knobOpts.index(knob) 
   
  for i in nodes:
    knobdata =  i.writeKnobs(nuke.TO_SCRIPT)

    #Checks the knob to see if it has a bbox knob
    regPad = re.compile('bbox (A|B|union|intersection)', re.IGNORECASE)
    regMatch =  regPad.search(knobdata)
    if regMatch: 
#      print 'Mult-Match'
#      print regMatch.group(1)
      i.knob("bbox").setValue(knobSetValue)
  
  return
    
