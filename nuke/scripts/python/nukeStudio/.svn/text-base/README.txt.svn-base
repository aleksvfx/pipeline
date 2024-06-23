README.txt

INSTALL:
-------------------------------------------------------------------------------------

please add this code to the init.py located in nuke distribution plugin folder:
.../Nuke5.1.0/plugins/init.py

#######################################################
#Add Nuke Studio Custom
#
try:
  NUKE_STUDIO_CUSTOM = os.environ["NUKE_STUDIO_CUSTOM"]
except:
  if os.path.isdir('/mnt/S2/3d/farm/nuke/studio_custom'):
    os.environ["NUKE_STUDIO_CUSTOM"] = "/mnt/S2/3d/farm/nuke/studio_custom"
  elif os.path.isdir('/S2/3d/farm/nuke/studio_custom'):
    os.environ["NUKE_STUDIO_CUSTOM"] = "/S2/3d/farm/nuke/studio_custom"
  else:
    print "Studio Custom Not Found"


NUKE_STUDIO_CUSTOM = os.environ["NUKE_STUDIO_CUSTOM"]
nuke.pluginAddPath(NUKE_STUDIO_CUSTOM)

#nuke.pluginAddPath("/mnt/S2/3d/farm/nuke/studio_custom")
#
#######################################################

