import nuke
import os

def disEnvVars ():
  envVarInfo  = ""

  envVars = os.environ.keys()
  envVars.sort()
  
  for param in envVars:
    # print "%20s %s" % (param,os.environ[param])
    envVarInfo += param +":  " + os.environ[param] + "\n" 

  p = nuke.Panel("Environment Variables")
  p.addNotepad("", envVarInfo)
  p.setWidth(600)
  result = p.show()
  return
  
