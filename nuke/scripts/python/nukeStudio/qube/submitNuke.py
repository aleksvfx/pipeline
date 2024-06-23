#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 
import sys



#########################################################
#    Qube Submit to farm Nuke
#########################################################


def submitNuke (style = 'basic'):

  import nuke 
  import os, shutil
  from nukeStudio import userSettings
  import platform
  import re
  from nukeStudio.structure import fixpath

  qubeExecutable = '/S2/3d/farm/nuke/apps/software/Nuke5_lin/Nuke5'
  if ( 'JOB_' in os.environ )  and ( 'SHOT_' in os.environ ):
     print "INFO: Using setshot environment."
     nuke_ver_major = int (nuke.NUKE_VERSION_MAJOR)
     nuke_ver_minor = int (nuke.NUKE_VERSION_MINOR)
     nuke_ver_rel = int (nuke.NUKE_VERSION_RELEASE)
     # e.g. /tools/apps/nuke/Nuke5.1v5_linux_64/Nuke5.1
     # nuke_exe = "/tools/apps/nuke/Nuke%d.%dv%d_linux_64/Nuke%d.%d" % (
     #    nuke_ver_major, nuke_ver_minor, nuke_ver_rel, nuke_ver_major, nuke_ver_minor)

     nuke_exe = "/tools/apps/nuke/nuke_%d.%dv%d_linux_64/Nuke%d.%d" % ( nuke_ver_major, nuke_ver_minor, nuke_ver_rel, nuke_ver_major, nuke_ver_minor)

     # nuke_exe = "/tools/apps/nuke/Nuke5.1v5_linux_64/Nuke5.1"
     qubeExecutable = 'source /tools/release/bin/setshotcmd %s/%s; %s' % ( os.environ['JOB_'], os.environ['SHOT_'], nuke_exe)
  #qubeExecutable = '/S2/3d/farm/nuke/apps/software/builds/Nuke5.1.2_lin_32/Nuke5.1'
  #qubeExecutable = 'nuke5'
  #qubeExecutableArgs = '-x -i'
  qubeExecutableArgs = '-x'
  qubeJobType = 'cmdrange'

  jobScript = nuke.knob('root.name')

  #get the output directory
  write_nodes = []
  for node in nuke.allNodes():
     if node.Class() == "Write":
         if node.knob('disable').getValue() == False: 
             write_nodes.append(node)
          
  outputFile = nuke.filename(write_nodes[0])
  if outputFile:
      outputDir = os.path.dirname(outputFile)
      
  
  

  if jobScript == None:
    jobName = ''
  else:
    jobName = os.path.basename(jobScript)

  jobFrameRange = nuke.knob('root.first_frame')+'-' + nuke.knob('root.last_frame')
  jobStepFrame = ''
  jobUser = userSettings.user
  jobEmail = '0'
  jobPriority = '101'   #ADD: studio default look up
  jobCPUs = '5'   #ADD: find out how many Nuke nodes are currently aviable
  jobGroups = 'nuke'

  #Detailed Submit options
  jobHosts = ''
  jobOmitHosts = ''
  jobGroups = 'nuke'
  jobCluster = ''
  jobRequirements = ''
  jobReservations = 'host.processors=1+'
  jobDependencies = ''
  jobNotes = ''
  
  ####################################################
  #BUILD PANEL
  ####################################################
  
  submitAsOpts = 'Pending Blocked'

  if style == 'basic':
    p = nuke.Panel("QUBE: Submit Basic")
  elif style == 'detail':
    p = nuke.Panel("QUBE: Submit Detail")
  else:
    return "ERROR: Panel type"
  
  p.addSingleLineInput('Name:', jobName)
  if jobScript == '':
    p.addFilenameSearch('ScriptFile:', jobScript)
  p.addSingleLineInput('Priority:', jobPriority)
  p.addSingleLineInput('CPUs:', jobCPUs)
  p.addSingleLineInput('Range:', jobFrameRange)
  if style == 'detail':
    p.addSingleLineInput('Step Frame:', jobStepFrame)
  
  ##ADD Later
  #p.addEnumerationPulldown('Execute': 'All Write1 Write2 Write3')
  #p.addSingleLineInput('Cash Size': jobCashSize)
  #p.addEnumerationPulldown('Render Mode': 'Full Proxy')
  #p.addSingleLineInput('Threads': jobThreads)
  #p.addBooleanCheckBox('Verbose': '0')
  

  p.addSingleLineInput('User:', jobUser)
  if style == 'detail':
    p.addBooleanCheckBox('Email:', jobEmail)
  p.addEnumerationPulldown('Submit as:', submitAsOpts)

  if style == 'detail':
    p.addSingleLineInput('Hosts:', jobHosts)
    p.addSingleLineInput('Omit Hosts:', jobOmitHosts)
    p.addSingleLineInput('Groups:', jobOmitHosts)
    p.addSingleLineInput('Cluster:', jobCluster)
    p.addSingleLineInput('Requirements:', jobCluster)
    p.addSingleLineInput('Reservations:', jobReservations)
    p.addSingleLineInput('Dependencies:', jobDependencies)
  
    p.addNotepad('Notes:', jobNotes)
  

  p.addButton("Cancel") # returns 0
  p.addButton("Submit") # returns 1
  p.setWidth(300)
  
  #Show Panel
  try: 
    result = p.show()
  except:
    result = False
  
  #If Canceled EXIT 
  if result == False:
    #nuke.message('Cancled')
    return
  
  jobName = p.value("Name:")
  if jobName == '':
    jobName == 'Untitled'
  jobName = "\'[2D] " + jobName + "\'"
  
  if jobScript == '':
    jobScript = p.value("ScriptFile:")
  
  jobPriority = p.value("Priority:")
  jobCPUs = p.value("CPUs:")
  jobFrameRange = p.value("Range:")
  jobUser = p.value("User:")
  jobStatus = p.value("Submit as:")
  
  if style == 'detail':
    pass
    #jobEmail = p.value("Email:")
    #jobStepFrame = p.value("Step Frame:")
    #jobHosts = p.value("Hosts:")
    #jobOmitHosts = p.value("Omit Hosts:")
    #jobGroups = p.value("Groups:")
    #jobCluster = p.value("Cluster:")
    #jobRequirements = p.value("Requirements:")
    #jobReservations = p.value("Reservations:")
    #jobDependencies = p.value("Dependencies:")
    #jobNotes = p.value("Notes:")


  
  
  ####################################################
  #Write Node clean up 
  ####################################################
  ###FIX ME: Make work correctly with fixpath

  if nuke.toNode('root').knob('proxy').getValue() == False:
    writeKnob  = 'file'
  else: 
    writeKnob = 'proxy'
  
  all_nodes = nuke.allNodes()
  print '\n'
  for node in all_nodes:
      if node.Class() == 'Write':
          #print node.name()
          if node.knob('disable').getValue() == False: 
              #path = node.knob('file').getValue()
              path = nuke.filename (node)
              if (not path):
                  node.knob ('disable').setValue (True)
                  msg = "Warning, your write node (%s) doesn't have a path for %s.\nWe've disabled the node" % (node.name (), writeKnob)
                  nuke.message (msg)
                  continue
              path = fixpath(path, 'Linux')
              node.knob('file').setValue(path)


  ##SAVE UPDATE
  nuke.scriptSave()

  ####################################################
  #Qube Data Setup and Submit 
  ####################################################
  
  #FIX PATH STRUCTURE 
  #if platform.system() == 'Darwin':
    #regPad = re.compile('/Volumes/s2/', re.IGNORECASE)
    #jobScript =  re.sub(regPad, '/S2/3d/', jobScript)
  
  jobScript = fixpath(jobScript)
  
  qubeJobData = '\'(=(cmdline=' + qubeExecutable + ' ' + qubeExecutableArgs + ' ' + jobScript + ' QB_FRAME_NUMBER)(range=' + jobFrameRange + ')\''

  if platform.system() == 'Darwin':
     cmd = '/Applications/pfx/qube/bin/qbsub'
  else:
     cmd = 'qbsub'

  cmd += ' --type ' + qubeJobType 
  cmd += ' --data ' + qubeJobData
  cmd += ' --range ' + jobFrameRange
  cmd += ' --name ' + jobName
  cmd += ' --priority ' + jobPriority
  cmd += ' --cpus ' + jobCPUs
  cmd += ' --groups ' + jobGroups
  cmd += ' --reservations ' + jobReservations
  if jobStatus == 'Blocked':
    cmd += ' --status blocked'
  
  #QUBE SUBMIT STRING
  #cmd = "qbsub --type cmdrange --data '(=(cmdline=nuke -x /S2/pers/cbankoff/testing/Qube_Test_v01.nk QB_FRAME_NUMBER)(range=100-200)' --range 100-200 --name Nuke_Qube_v11 --priority 100 --cpus 5 --groups nuke --reservations host.processors=4 --status blocked"
  
  
  (fd_std_out, fd_std_in, fd_std_error) =  os.popen3(cmd)
  
  #copy nuke script to image output folder
  if not os.path.isfile(os.path.join(outputDir, jobName)):
      print("copying nuke script to %s" % outputDir)
      shutil.copy(jobScript,outputDir)
  
  #Return Stucces
  
  qubeReturn = fd_std_in.read()
  
  regErrorPattern = re.compile('error', re.IGNORECASE)
  regJobNumberPattern = re.compile('^[0-9]*')
  regFindOutputPathPattern = re.compile('')
  
  if regErrorPattern.search(qubeReturn):
    nuke.message('ERROR(Qube):'+ qubeReturn + '\n\n\n'+cmd)
    return
  
  #qubeJobeNumber = regJobNumberPattern.search(qubeReturn)
    
  if regJobNumberPattern.search(qubeReturn):
    nuke.message(cmd+"\n\nQUBE JOB NUMBER: " + qubeReturn)
    #ADD: Log the qube job number with the shot.
    return
  
  nuke.message(cmd+"\n\nERROR: " + qubeReturn)
  return
    
    
    
    
