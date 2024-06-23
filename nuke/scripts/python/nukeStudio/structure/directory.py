 
#  Copyright (c) Chris Bankoff.  All Rights Reserved
#
# 

#########################################################
#    Studio Directory Structure
#########################################################

#DEPERCATED: Being replaced by fileStructure.py
#DEPERCATED:
#DEPERCATED:

#DEPERCATED:
ROOT_JOB = '/S2/proj/'
ROOT_RENDERS = '/S3/prod/'
ROOT_FRAMES = '/S3/prod/'

#DEPERCATED:
ROOT_NETWORK_USERS = '/S2/pers/'

#DEPERCATED:
class ShotStructure:
  'Directory structure for shots'

#DEPERCATED:  
  def __init__(self, __shot = 'aa000'):
    'set up the variables based on a shot'
    #self.FOO = ROOT_JOB
    self.SHOT = __shot

#DEPERCATED:    
    #self.COMP_SCRIPTS = ROOT_JOB + self.SHOT



#foo = nukeStudio.structure.directory.ShotStructure()
#print foo.SHOT