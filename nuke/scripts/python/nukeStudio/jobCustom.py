#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 


#########################################################
#    JOB SETUP
######################################################### 
from . import NUKESTUDIO_PACKAGE_LOCATION

DEFAULT_JOB_LOC = NUKESTUDIO_PACKAGE_LOCATION + "/defaultJob"


def loadJobCustom (JOB_CUSTOM_PATH):
  'load job custom from given location'

  import nuke
  import sys
  import os.path

  if os.path.exists(JOB_CUSTOM_PATH) == False:
      print "Path does not exists"
      return False

  if os.path.exists(JOB_CUSTOM_PATH +"/init.py") == False:
      print "init does not exists"
      return False

  #Add plugin Path
  nuke.pluginAddPath(JOB_CUSTOM_PATH)

  #Added Job Packages
  sys.path.append(JOB_CUSTOM_PATH +'site-packages')

  return True


def createJobCustom(JOB_CUSTOM_PATH):
  'Creates a Job Cutstom'
  import os
  import nuke
  from . import DEFAULT_JOB_LOC

  def copytree(src, dst):
      'recursive tree copy'

      from shutil import copy2
      
      #List all the files to copy
      names = os.listdir(src)
      
      #Tries to make the directory
      try:
        os.mkdir(dst)
      except:
        pass
      
      #Copy Loop
      for name in names:
      
          #Make fill path
          srcname = os.path.join(src, name)
          dstname = os.path.join(dst, name)
          
          #Try to execute copy
          try:
              #If file is a link
              if os.path.islink(srcname):
                  linkto = os.readlink(srcname)
                  os.symlink(linkto, dstname)
              #If it is a dir then run copytree
              elif os.path.isdir(srcname):
                  copytree(srcname, dstname)
              #Copy file
              else:
                  copy2(srcname, dstname)
          #error handleing
          except (IOError, os.error), why:
              print "Can't copy %s to %s: %s" % (`srcname`, `dstname`, str(why))
      return  

  #make directory path
  try:
    os.makedirs(JOB_CUSTOM_PATH)
  except OSError:
    print "File already Exists"
    pass

  try:
   copytree(DEFAULT_JOB_LOC, JOB_CUSTOM_PATH)
  except:
    return False
  
  return True

def jobCustomInit(JOB_CUSTOM_PATH):
  'init job custom'

  import nuke  
  from . import DEFAULT_JOB_LOC
  from . import createJobCustom
  from . import loadJobCustom
  
  errorCreateMessage = 'ERROR(jobCustom): Job custom not found\n' + JOB_CUSTOM_PATH + '\n\nWould you like to create it?\n' 
  errorMessage = "ERROR(jobCustom): Unable to load custom: "

  if loadJobCustom(JOB_CUSTOM_PATH) == False:
  
    #ask the user if he wants to create a job custom 
    print "JOB CUSTOM NOT FOUND: " + JOB_CUSTOM_PATH
    if nuke.ask(errorCreateMessage):
    
      #create job custom
      print "CREATING JOB CUSTOM: " + JOB_CUSTOM_PATH
      if createJobCustom(JOB_CUSTOM_PATH):
        if loadJobCustom(JOB_CUSTOM_PATH):
          print 'JOB CUSTOM LOADED: '+ JOB_CUSTOM_PATH
        else:
          nuke.message(errorMessage + JOB_CUSTOM_PATH)
      else:
        nuke.message(errorMessage + JOB_CUSTOM_PATH)
    
    #else load the default job custom
    else:
      JOB_CUSTOM_PATH = DEFAULT_JOB_LOC
      if loadJobCustom(JOB_CUSTOM_PATH):
        print "LOADING DEFAULT JOB: "+ JOB_CUSTOM_PATH
      else:
        nuke.message(errorMessage + JOB_CUSTOM_PATH)
  else:
    print 'JOB CUSTOM LOADED: '+ JOB_CUSTOM_PATH
  
  return
  


