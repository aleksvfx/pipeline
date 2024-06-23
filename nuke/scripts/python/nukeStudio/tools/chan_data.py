
#DataTypes

def nodeChannelKnobs(Node):
  'search a node and return a list of knobs to import or export channel data'

  import nuke
  import re

  knobsData = nuke.toNode(Node).writeKnobs(1)
  knobIndex = nuke.toNode(Node).writeKnobs(0)

  #print knobIndex
  #print knobsData
  #print "-------------------------------------------------------"

  knobListReturn = []

  knobsList = knobIndex.split(' ')
  for knob in knobsList:
      #List of knobs to ignore
      ignoreList = ['display', 'selectable', 'import_chan', 'export_chan', 'xform_order', 'rot_order', 'projection_mode']

      #IF in the pass list then continue to the next knob
      if knob in ignoreList:
          #print "PASS"
          continue

      knobs3D = ['translate', 'rotate', 'scaling', 'skew', 'pivot', 'near']

      if knob in knobs3D:
          #verify that is a 3D knob
          regExt = re.compile(knob+' \{[0-9]* [0-9]* [0-9]*\}')
          regMatch = regExt.search(knobsData)

          #If is is a 3D knob then add it to return list and continue
          if regMatch != None:
              #print regMatch.group(0)
              knobListReturn.append(knob+'.x')
              knobListReturn.append(knob+'.y')
              knobListReturn.append(knob+'.z')
              continue

      knobsUV = ['win_translate', 'rotate', 'scaling', 'skew', 'pivot', 'near']

      if knob in knobsUV:
          #verify that is a 3D knob
          regExt = re.compile(knob+' \{[0-9]* [0-9]*\}')
          regMatch = regExt.search(knobsData)

          #If is is a 3D knob then add it to return list and continue
          if regMatch != None:
              #print regMatch.group(0)
              knobListReturn.append(knob+'.u')
              knobListReturn.append(knob+'.v')
              continue

      #print knob
      knobListReturn.append(knob)

  #print knobListReturn

  return knobListReturn


def importDataConversion(dataTypes):
  import nuke
  
  print str(dataTypes)
  
  p = nuke.Panel("Import Data Type:")
  
  returnLookUp = []
  
  if 'rotate.x' in dataTypes or 'rotate.y' in dataTypes or 'rotate.z' in dataTypes:
    #Add rotation to the returnLookUp list  
    returnLookUp.append('rotation')
    #add to dropdown to the panel
    p.addEnumerationPulldown('rotation', 'degrees radians')
  
  cameraKnobList = ['haperture', 'vaperture','focal']  
  dataFormatOpts = "mm cm inch"
  
  #Add Camera Data options
  for knobType in cameraKnobList:
    try:
      #Look for the data type
      dataTypes.index(knobType)
      
      #if found add to to the returnLookUp List
      returnLookUp.append(knobType)
      
      #Add pulldown option to the panel
      p.addEnumerationPulldown(knobType, dataFormatOpts)
    except:
      pass
  
  #add buttons and window size
  p.addButton("Cancel") # returns 0
  p.addButton("Ok") # returns 1
  p.setWidth(250)  
  
  if returnLookUp != []:  
    try: 
      result = p.show()
    except:
      result = False
  else:
    result = True
    
  if result:
    if 'rotation' in returnLookUp:
      if p.value('rotation') == 'radians':
        rotate_converstion = 57.2957795
      else: 
        rotate_converstion = 1
    else:
      rotate_converstion = 1
    
    if 'focal' in returnLookUp:
      if p.value('focal') == 'inch':
        focal_converstion = 25.4
      elif p.value('focal') == 'cm':
        focal_converstion = 100
      else:
        focal_converstion = 1
    else:
      focal_converstion = 1

    if 'haperture' in returnLookUp:
      if p.value('haperture') == 'inch':
        haperture_converstion = 25.4
      elif p.value('haperture') == 'cm':
        haperture_converstion = 100
      else:
        haperture_converstion = 1
    else:
      haperture_converstion = 1      
          
    if 'vaperture' in returnLookUp:
      if p.value('vaperture') == 'inch':
        vaperture_converstion = 25.4
      elif p.value('vaperture') == 'cm':
        vaperture_converstion = 100
      else:
        vaperture_converstion = 1
    else:
      vaperture_converstion = 1      

    print 'rotate_converstion: ' + str(rotate_converstion)
    print 'focal_converstion: ' + str(focal_converstion)
    print 'haperture_converstion: ' + str(haperture_converstion)
    print 'vaperture_converstion: ' + str(vaperture_converstion)
    
    return rotate_converstion, focal_converstion, haperture_converstion, vaperture_converstion 
    
  else:
    return False
  

def channelOrderPanel(numberOfDataTypes, dataTypes = ''):
  'opens a nuke panel for setting chan data types'
  
  import nuke
  #from nukeStudio.tools.chan_data import nodeChannelKnobs
  
  #print nuke.toNode('Camera1').writeKnobs(1)
  
  if dataTypes == '':
    dataTypes = ["none", "frame" "translate.x", "translate.y", "translate.z", "rotate.x", "rotate.y", "rotate.z", "scaling.x", "scaling.y", "scaling.z", "focal", "haperture", "vaperture"]
  else:
    dataTypes = ['none', 'frame'] + dataTypes
  
  dataTypeOpts = ""
  for dataOpt in dataTypes:
    dataTypeOpts += dataOpt + ' '
  
  #Create Panel Options
  p = nuke.Panel("Import Chan Data:")
  
  #Create the number of pulldowns based on the numberOfNameTypes
  for chanNumber in range(numberOfDataTypes):
    p.addEnumerationPulldown(str(chanNumber), dataTypeOpts)
    
  #add buttons and window size
  p.addButton("Cancel") # returns 0
  p.addButton("Ok") # returns 1
  #p.setWidth(250)

  #Show Panelself.__iniFile
  try: 
    result = p.show()
  except:
    result = False
  
  #Crashes if value() is called and the panel was canceled 
  if result:
    return_data = []
    
    #get the data out of the pannel object
    for chanNumber in range(numberOfDataTypes):
      #add the data to the return_data list
      return_data.append(p.value(str(chanNumber)))

    #return the data order list
    return return_data
  else:
    return False

def createNodePanel():
  import nuke
 
  #Create Panel Options
  p = nuke.Panel("Create Node:")
  
  nodeTypeOpts = 'Camera Axis'
  
  p.addEnumerationPulldown("Type:", nodeTypeOpts)
  
  #add buttons and window size
  p.addButton("Cancel") # returns 0
  p.addButton("Ok") # returns 1
  #p.setWidth(250)

  #Show Panelself.__iniFile
  try: 
    result = p.show()
  except:
    result = False
  
  #Crashes if value() is called and the panel was canceled 
  if result:
    selected_node = nuke.createNode("Camera")
    #node_name = selected_node.name()
    
    return selected_node
  else:
    return None
  


def set_key(node, knob, frame, value):
  'work around function to set values at key frames'
  import nuke
  
  tclCmd = 'setkey ' + node + '.' + knob + ' ' + str(frame) + ' ' + str(value)
  try:
    #print tclCmd
    nuke.tcl(tclCmd)
  except:
    return 0
  return 1

def inspectChanFile (file):
  'looks at a chan file and return the number of data sets avaialbe based on the first line'

  #open File
  fd = open(file, 'r')
  
  #read first line
  inspectData = fd.readline() 
  
  #close file
  fd.close()

  numberDataSets = len(inspectData.split(' '))
  
  return numberDataSets
  

def file_import(chanFile = ''):
  'import chan data from a file'

  from nukeStudio.tools.chan_data import nodeChannelKnobs
  from nukeStudio.tools.chan_data import set_key
  from nukeStudio.tools.chan_data import inspectChanFile
  from nukeStudio.tools.chan_data import channelOrderPanel
  from nukeStudio.tools.chan_data import importDataConversion
  import nuke
  import os.path
  
 
  if chanFile == '':    
    #DEBUG:
    #chanFile = '/home/cbankoff/Desktop/nukeTestCam.chan'

    #Prompt User to select a chan file
    rawInput = nuke.getClipname('Select Chan File', '*.chan', chanFile)
  
    #Detect a promp cancel
    if rawInput == None: 
      print "QUIT"
      return
    else:
      chanFile = rawInput
      
  chanFileName = os.path.basename(chanFile)

  #inpsect the file to se the number of data sets
  numberOfDataSets = inspectChanFile(chanFile)

  #Create Empty Node
  #selected_node = nuke.createNode("Camera")
  selected_node = createNodePanel()  
  node_name = selected_node.name()

  #get a list of posible knobs
  nodeKnobList = nodeChannelKnobs(node_name)

  #Get list of data types in chan order
  knobLookupList = channelOrderPanel(numberOfDataSets, nodeKnobList)
  #knobLookupList = channelOrderPanel(numberOfDataSets)
  
  #DEBUG:
  #knobLookupList = ['translate.x', 'translate.y', 'translate.z', 'rotate.x', 'rotate.y', 'rotate.z', 'none', 'none', 'none', 'focal', 'haperture']

  #Set units of the data being imported
  rotate_converstion, focal_converstion, haperture_converstion, vaperture_converstion = importDataConversion(knobLookupList)

  fd = open(chanFile, 'r')

  frame = 0

  #try to to find the location of the frame number data
  try:
      frameDataLocation = knobLookupList.index('frame')
  except:
      frameDataLocation = -1

  #Make list of lines of data
  fileLineList = fd.readlines()

  #Read each line of data from chan file
  for fileLine in fileLineList:

    #split the data up into a list
    lineData = fileLine.split(' ')

    if frameDataLocation == -1:
      #incr frame number
      frame += 1
    else:
      frame = lineData[frameDataLocation]
          
    #for dataIndex in the number of elements in the list
    for dataIndex in range(len(lineData)):

      #get value from lineData
      value = lineData[dataIndex]

      #Look up the knob type
      knob = knobLookupList[dataIndex]
      
      
      #set knob value if not set to none
      if knob in ['none', 'frame']:
        #debugging
        print "PASS("+ str(frame) + "," + str(dataIndex) + ") "  + knob + ' ' + str(value) 
      else:
        #debugging
        print "("+ str(frame) + "," + str(dataIndex) + ") "  + knob + ' ' + str(value)      
        
        if knob == "haperture":
          set_key(node_name, knob, frame, float(value)*haperture_converstion)
          
        else:
          set_key(node_name, knob, frame, value)
     

  print 'done'
  fd.close()
  
  #Label the node with the name of the chan file
  selected_node.knob('label').setValue(chanFileName)

  return



def file_export(node = ''):
  'import chan data from a file'

  from nukeStudio.tools.chan_data import nodeChannelKnobs
  from nukeStudio.tools.chan_data import set_key
  from nukeStudio.tools.chan_data import inspectChanFile
  from nukeStudio.tools.chan_data import channelOrderPanel
  from nukeStudio.tools.chan_data import importDataConversion
  import nuke
  import os.path
  
  
  
  
  
  
  return
  
  



