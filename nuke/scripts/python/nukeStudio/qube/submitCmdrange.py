#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

##NOT WORKING!!!!!!!!!!!!!!!!!!!!

#########################################################
#    Qube Submit to farm Nuke
#########################################################


def submitCmdrange (qubeJobName, qubeExecutable, qubeExecutableArgs = '', jobFrameRange = '1-1', jobFramePadding ='0', mode = 'basic' ):

  import nuke 
#  from os.path import basename
  from nukeStudio import userSettings
  from os import popen3
  #import platform
#  import re
#  from nukeStudio.structure import fixpath
  from nukeStudio.structure.software import QBSUB

#  qubeExecutable = ''
#  qubeExecutableArgs = ''
  qubeJobType = 'cmdrange'


#  jobFrameRange = '1-1'
  #jobFramePadding = '0'
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

  if mode == 'basic':
    p = nuke.Panel("QUBE: Submit Basic")
  elif mode == 'detail':
    p = nuke.Panel("QUBE: Submit Detail")
  else:
    return "ERROR: Panel type"
  
  p.addSingleLineInput('Name:', qubeJobName)
  p.addSingleLineInput('Priority:', jobPriority)
  p.addSingleLineInput('CPUs:', jobCPUs)
  p.addSingleLineInput('Range:', jobFrameRange)
  #if style == 'detail':
    #p.addSingleLineInput('Step Frame:', jobStepFrame)
    
  p.addSingleLineInput('User:', jobUser)
  if mode == 'detail':
    p.addBooleanCheckBox('Email:', jobEmail)
  p.addEnumerationPulldown('Submit as:', submitAsOpts)

  if mode == 'detail':
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
  
  #Show Panel/S2/3d/prod/testxxM000/02_production/Vray_Dev_01.mpj/images/EVO/cam13/EVO_cam13_v01.%04d.exr
  try:
    #if mode in ['detail', 'ask']: 
      #result = p.show()
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
  
  #if jobScript == '':
    #jobScript = p.value("ScriptFile:")
  
  jobPriority = p.value("Priority:")
  jobCPUs = p.value("CPUs:")
  jobFrameRange = p.value("Range:")
  jobUser = p.value("User:")
  jobStatus = p.value("Submit as:")
  
  if mode == 'detail':
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
  #Qube Data Setup and Submit 
  ####################################################

  
  qubeJobData = '\'(=(cmdline=' + qubeExecutable + ' ' + qubeExecutableArgs + ' )(range=' + jobFrameRange + ')(padding=' + jobFramePadding + ')\''

  #if platform.system() == 'Darwin':
     #cmd = '/Applications/pfx/qube/bin/qbsub'
  #else:
     #cmd = 'qbsub'
  cmd = QBSUB

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
  
  
  (fd_std_out, fd_std_in, fd_std_error) =  popen3(cmd)
  
  #Return Stucces
  
  nuke.message(cmd+"\n\nQUBE JOB NUMBER: " +fd_std_in.read())
  
  return
    


####
#VRIMG2EXR = '/S2/3d/farm/nuke/apps/software/vrimg2exr/vrimg2exr_x86/vrimg2exr.exe'

#submitCmdrange('Test_cmd_v01', VRIMG2EXR, '-h')



    
    