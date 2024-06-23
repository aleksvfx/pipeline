import nuke

def disVars ():
  varsDic = vars()
  info  = ""
  indexKeys = varsDic.keys()
  indexKeys.sort()
  
  for param in indexKeys:
      info += param +":  " + str(varsDic[param]) + "\n"
  
  p = nuke.Panel("Variable Name Space")
  p.addNotepad("", info)
  p.setWidth(600)
  result = p.show()
  return

 
