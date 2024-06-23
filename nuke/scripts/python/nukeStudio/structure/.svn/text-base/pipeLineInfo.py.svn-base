#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 


#########################################################
#    Pipe Line Info
#
#Tool will later be upgrated to talk to the Mikes data bases
#For know it will just come directory strucutres
#########################################################

# FIX ME:
############
# 1. Clean up script
#
#

# TO DO:
############
# 1. Need to create a job name slicer
#
#


def jobs():
  'returns the curent job list'
  #Add Status Control
   
  from os import listdir
  from os.path import isdir
  from nukeStudio.structure import currentStructure
  
  #FIX ME
  #Need to find the job list from data base
  
  
  jobFolodersPath = currentStructure.get('ROOT_JOB')
  
  jobDirList = listdir(jobFolodersPath)
  
  jobList = []
  
  for f in jobDirList:
    fpath = jobFolodersPath + '/' + f
    
    if isdir(fpath):
      
      jobList.append(f)
  
  jobList.sort()
  
  return jobList 


def users():
  'returns a list of users' 
  
  from os import listdir
  from os.path import isdir
  from nukeStudio.structure import currentStructure
  
  userFolodersPath = currentStructure.get('ROOT_USER_FOLDERS')
  
  userDirList = listdir(userFolodersPath)
  
  userList = []
  
  for f in userDirList:
    fpath = userFolodersPath + '/' + f
    if isdir(fpath):
      userList.append(f)
  
  userList.sort()
  
  return userList 

def shots():
  'a list of shots for the current shots' 
  
  #from nukeStudio.structure import directory 
  from os import listdir
  from os.path import isdir
  from nukeStudio import JOB_DIR
  from nukeStudio.structure import currentStructure
  
  shotFolodersPath = currentStructure.get('JOB_SHOTS_PATH')
  
  #JOB_SHOTS = directory.ROOT_JOB + '/' + JOB_DIR +'/' + '/production/'
  
  try:
    #shotDirList = listdir(JOB_SHOTS)
    shotDirList = listdir(shotFolodersPath)
  except:
    return None
  
  shotList = []
  
  for f in shotDirList:
    fpath = shotFolodersPath + '/' + f
    if isdir(fpath):
      shotList.append(f)
  
  shotList.sort()
  
  return shotList 














