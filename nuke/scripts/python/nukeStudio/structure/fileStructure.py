#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

#########################################################
#    File Structure Class
#########################################################
 
class fileStructure():
  def __init__(self, platformOS = ''):
    import platform 
    #import nuke
    
    #For return what platoform the system currently suports. 
    self.platformTypes = ('Linux', 'Darwin')
    
    #List used for fixpath loook up
    self.rootList = ('ROOT_JOB', 'ROOT_RENDERS', 'ROOT_FRAMES', 'ROOT_PLATES', 'ROOT_USER_FOLDERS')
    
    if platformOS == '':
      platformOS = platform.system()
      #nuke.message(platformOS)
    
    self.platform = platformOS
    
    self.__dataStructure = {}
    
    #Defaults
    self.update('ROOT_JOB', '/S2/3d/prod/')
    self.update('ROOT_RENDERS','/S3/prod/')
    self.update('ROOT_FRAMES', self.__dataStructure.get('ROOT_RENDERS'))
    self.update('ROOT_PLATES', self.__dataStructure.get('ROOT_RENDERS'))
    self.update('ROOT_USER_FOLDERS','/S2/pers/')
    self.update('NETWORK_FONTS', '/S2/3d/farm/nuke/studio_custom/font/')

    #Over Rides
    if platformOS == 'Darwin':
      self.update('ROOT_JOB', '/Volumes/s2/prod')
      self.update('ROOT_RENDERS','/Volumes/s3/')
      self.update('ROOT_FRAMES', self.__dataStructure.get('ROOT_RENDERS'))
      self.update('ROOT_PLATES', self.__dataStructure.get('ROOT_RENDERS'))
      self.update('ROOT_USER_FOLDERS','/Volumes/s2/pers/')
      self.update('NETWORK_FONTS', '/Volumes/s2/farm/nuke/studio_custom/font/')
    
    return

  def __initUser__(self, user):
    
    USER = user
    USER_DIR = USER
    USER_PATH = self.__dataStructure.get('ROOT_USER_FOLDERS') + USER_DIR + "/"
    
    self.update('USER', USER)
    self.update('USER_DIR', USER_DIR)
    self.update('USER_PATH', USER_PATH)
    return 
  
  def __initJob__(self, job_dir):
    
    JOB_DIR = job_dir
    JOB = JOB_DIR
    JOB_PATH = self.__dataStructure.get('ROOT_JOB') + JOB_DIR + "/"
    RENDERS_PATH = self.__dataStructure.get('ROOT_RENDERS') + JOB_DIR + "/"
    
    JOB_SHOTS_DIR = 'production'
    JOB_SHOTS_PATH = JOB_PATH + '/' + JOB_SHOTS_DIR + "/"
    
    RENDERS_SHOT_DIR = 'production'
    RENDERS_SHOT_PATH = RENDERS_PATH + '/' + RENDERS_SHOT_DIR + "/"
    
    JOB_CUSTOM_DIR = 'resources/apps/nuke'
    JOB_CUSTOM_PATH = JOB_PATH + JOB_CUSTOM_DIR + '/'
    
    self.update('JOB', JOB)
    self.update('JOB_DIR', JOB_DIR)
    self.update('JOB_PATH', JOB_PATH)
    
    self.update('JOB_SHOTS_DIR', JOB_SHOTS_DIR)
    self.update('JOB_SHOTS_PATH', JOB_SHOTS_PATH)
    
    self.update('RENDERS_SHOT_DIR', RENDERS_SHOT_DIR)
    self.update('RENDERS_SHOT_PATH', RENDERS_SHOT_PATH)
    
    self.update('JOB_CUSTOM_DIR', JOB_CUSTOM_DIR)
    self.update('JOB_CUSTOM_PATH', JOB_CUSTOM_PATH)


    return 
    
  def __initShot__(self, shot):
    
    SHOT = shot
    SHOT_DIR = SHOT
    #SHOT_PATH = self.__dataStructure.get('JOB_PATH') + 'production/' + SHOT_DIR + "/"
    SHOT_PATH = self.__dataStructure.get('JOB_SHOTS_PATH') + SHOT_DIR + "/"
    
    COMP_SCRIPTS_DIR = 'comp'
    COMP_SCRIPTS_PATH = SHOT_PATH + "2d/" + COMP_SCRIPTS_DIR + "/"
    
    self.update('SHOT', SHOT)
    self.update('SHOT_DIR', SHOT_DIR)
    self.update('SHOT_PATH', SHOT_PATH)
    
    self.update('COMP_SCRIPTS_DIR', COMP_SCRIPTS_DIR)
    self.update('COMP_SCRIPTS_PATH', COMP_SCRIPTS_PATH)
    
    return

  def update(self,MetaTag, Data):
    self.__dataStructure.update({MetaTag:Data})
    return
  
  def keys(self):
    return self.__dataStructure.keys()
  
  def get(self,args):
    try: 
      return self.__dataStructure[args]
    except:
      raise 'error'
  
  def setShot(self, shot):
    self.__initShot__(shot)
    return True
  
  def setJob(self, job):
    self.__initJob__(job)
    return True
  
  def setUser(self, user):
    self.__initUser__(user)
    return True

  def setUser(self, user):
    self.__initUser__(user)
    return True


def fixpath (path, platform = ''):
  
  import platform
  from nukeStudio.structure import fileStructure
  import re

  #the file strcuture the we are try to conform to
  replaceStructure = fileStructure(platform) 
  
  endsearch = 0
  
  for testPlatform in replaceStructure.platformTypes:
    
    #Don't test to see if already matches the file structure we are trying to confrom to
    #move on an test the next platform type
    if testPlatform == platform:
      #print 'testPlatform Bypass: ' + testPlatform
      continue
    
    #create a test strcture based on a suported platform
    tempStructure = fileStructure(testPlatform)
    #print 'testing: ' + testPlatform
  
    #Look for a match based on root mnt points
    for mnt in  tempStructure.rootList:
      regP = '^' + tempStructure.get(mnt)
      if re.search(regP, path):
        #print 'Match: ' + mnt
        endsearch = 1
        break
  
    #completely break out of search
    if endsearch == 1:
      break
  
  #print 'the End'
  
  regS = replaceStructure.get(mnt)
  
  path =  re.sub(regP, regS, path)
  

  return path














#ROOT_JOB: The location of all the jobs setups
#ROOT_RENDERS: The location of all the jobs renders
#ROOT_FRAMES: The location of all the jobs finished frames
#ROOT_PLATES: The location of all the jobs plates

####

#JOB: Name of the Job
#JOB_DIR: Name of the Job Directoy
#JOB_PATH: Location and name of the Job Directory


#####

#SHOT: Name of the shot
#SHOT_DIR: Name of the shot directory
#SHOT_PATH: Location and the name of the Shot directroy

####

#SHOT_CAMERA_DIR:
#SHOT_CAMERA_PATH:

#####

#COMP_SCRIPTS_DIR: name of the comp scripts
#COMP_SCRIPTS_PATH: Location and the name of the shot direcroy

#PRECOMP_SCRIPTS_DIR: name of the pre comp scripts
#PRECOMP_SCRIPTS_PATH: Location and the name of the shot direcroy

#LTCOMP_SCRIPTS_DIR: name of the lighting comp scripts
#LTCOMP_SCRIPTS_PATH: Location and the name of the shot direcroy

#ROTO_SCRIPTS_DIR: name of the roto scripts
#ROTO_SCRIPTS_PATH: Location and the name

#####

#COMP_FRAMES_DIR: name of the comp frames
#COMP_FRAMES_PATH: Location and the name 

#PRECOMP_FRAMES_DIR: name of the pre comp frames
#PRECOMP_FRAMES_PATH: Location and the name 

#LTCOMP_FRAMES_DIR: name of the lighting comp frames
#LTCOMP_FRAMES_PATH: Location and the name 

#ROTO_FRAMES_DIR: name of the roto frames
#ROTO_FRAMES_PATH: Location and the name


