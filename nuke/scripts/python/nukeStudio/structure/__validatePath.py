def validatePath(path, mode = 'ask'):
  'Given a path and valiateds if it exists regarless of patform;  return True or false; If it does not exists, can propt user to create it.'
   
  if os.path.exists(path):
    return true
  
  else:
      
from nukeStudio.structure import fixpath

path='/Volumes/s2/pers/cbankoff/osx_render_v06/osx_render_v06.0001.tga'
#path='/S2/pers/cbankoff/osx_render_v06/osx_render_v06.0001.tga'

print path
print fixpath(path, 'Darwin')




