#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

def vrimg2exr (imageLoc = '/') :
  'Convert vray image files to exrs'

  import nuke
  import os
  import re
  import nukeStudio
  from nukeStudio.qube import submitCmdrange
  from nukeStudio.structure.software import VRIMG2EXR_32
  from nukeStudio.structure.software import VRIMG2EXR_64
  from os.path import basename

  conversionArgs = ''
  
  def convertFrame (frame, conversionArgs, bit='32'):
    #handles the convertion exe
    from nukeStudio.structure import software
    from nuke import message
    import re
    
    #Need the studio.software to location for VRIMG2EXR 
    try:
      program = software.VRIMG2EXR
    except: 
      raise NameError, 'ERROR(structure): VRIMG2EXR'
    
    if os.path.exists(software.VRIMG2EXR) == False:
        raise NameError, 'ERROR(VRIMG2EXR): file not found'
    
    #subes the .vrimg extention for exr
    regExt = re.compile('.vrimg$')
    exrFrames = regExt.sub('.exr', frame)

    cmd = program + " \'" + framePath + "\' \'" + exrFrames + "\' " +conversionArgs
    
    #debug: 
    message( cmd+'\n' )
    
    (fd_std_in, fd_std_out) =  os.popen4(cmd)
    print fd_std_out.read()
    #ADD: Debuged Return from vrimg2exr to makesure it was successful
    
    return 
  
  def convertOptions ():
    #Generate all the convertion args
    
    bitOpt = '16bit 32bit'
    colorOpt = 'Linear sRGB'
    cropOpt = 'Auto None'
    compressOpt = 'zips zip piz pxr24 none'
    
    conversionArgs = ''
    
    p = nuke.Panel ('vRay to Exr')
    p.setWidth(200)
    p.addEnumerationPulldown('Bit Depth:', bitOpt)
    p.addEnumerationPulldown('Color Space:', colorOpt)
    p.addSingleLineInput('Gamma:', 1.0)
    p.addEnumerationPulldown('Channels:', 'All')
    p.addEnumerationPulldown('Crop:', cropOpt)
    p.addEnumerationPulldown('Compression:', compressOpt)
    p.addButton ('Cancel') #Returns 0
    p.addButton ('Convert')  #Returns 1 
    result = p.show()
    
    if result == 1:
        if p.value('Bit Depth:') == '16bit':
            conversionArgs += '-half '
        if p.value('Color Space:') == 'sRGB':
            conversionArgs += '-sRGB '
        if p.value('Gamma:') != '1.0':
            gamma = p.value('Gamma:')
            conversionArgs += '-gamma  ' + gamma
        if p.value('Channels:') != 'All':
            pass
            #FIX ME: Add selected channel support
        if p.value('Crop:') != 'Auto':
            conversionArgs += '-dataWindow'
        compression = p.value('Compression:')
        conversionArgs += '-compression ' + compression + ' ' 
        return conversionArgs
    else:
      return 0

 
  #Prompt User to select a vRay images
  rawInput = nuke.getClipname('Select Package', '*.vrimg', imageLoc)
  
  #Detect a promp cancel
  if rawInput == None: 
    print "QUIT"
    return

  regFrameRange = re.compile('(.*) ([0-9]*)-([0-9]*)$')
  matchFrameRange = regFrameRange.search(rawInput)

  if matchFrameRange != None:
      #print 'Match'
      #print matchFrameRange.group(0)
      pathImages = matchFrameRange.group(1)
      firstFrame = int(matchFrameRange.group(2))
      lastFrame = int(matchFrameRange.group(3))
      frameRange = str(firstFrame) + '-' + str(lastFrame)
  else:
      #print 'No Match'
      frameRange = '0-0'
      firstFrame = 0
      lastFrame = 0

  regPad = re.compile('%[0-9]([0-9])d')
  regMatch =  regPad.search(pathImages)
    
  result = convertOptions()
  if result == 0:
    return
  else:
    conversionArgs = result
  
  #render qube
  exeOpt = '32bit 64bit'
  renderLocOpt = 'Farm Local'
  
  p = nuke.Panel ('Render '+ frameRange)
  p.setWidth(200)
  p.addEnumerationPulldown('Converter Type:', exeOpt)
  p.addEnumerationPulldown('Location:', renderLocOpt)
  
  result = p.show()
  
  if result == 1:
    exeType = p.value('Converter Type:')
    renderLoc = p.value('Location:')
  else:
    return 0

  if exeType == '64bit':
    renderExec = VRIMG2EXR_64
  else:
    renderExec = VRIMG2EXR_32

  if renderLoc == 'Farm':
    jobName = basename(pathImages)
    framePad = regMatch.group(1)
    
    vrayFramePath = regPad.sub('QB_FRAME_NUMBER', pathImages)
    
    regExt = re.compile('.vrimg$')
    exrFramePath = regExt.sub('.exr', vrayFramePath)

    #print jobName + ' ' + renderExec + ' ' + vrayFramePath + ' ' + exrFramePath + ' ' +conversionArgs + ' ' + frameRange + ' ' + framePad
    
    renderExecArgs = vrayFramePath + ' ' + exrFramePath + ' ' + conversionArgs 
    
    submitCmdrange(jobName, renderExec, renderExecArgs, frameRange, framePad)
  
  else:
    #if match is found then multiple frames else single frame conversion
    if regMatch:
      
      pad = regMatch.group(1)
      frmpd = "%0" + str(pad) + "d"
      #print  p.sub('$$$$', pathImages)
      
      #Contvert the frames
      for frame in range(firstFrame,lastFrame+1):
        framePath = regPad.sub(str(frmpd % frame), pathImages)
        
        convertFrame(framePath, conversionArgs)
    else:
      framePath = pathImages
      convertFrame(framePath, conversionArgs)
    
  regExt = re.compile('.vrimg$')
  exrFrames = regExt.sub('.exr', pathImages)

  nuke.createNode("Read", "file {"+exrFrames+"} first " + str(firstFrame) + " last " + str(lastFrame), inpanel = True)
  
  return
  
  
