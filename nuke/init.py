import nuke
import os
import sys

this_filename = sys._getframe().f_code.co_filename
print('Loading '+this_filename)

if not 'METHOD_TOOLS_ROOT' in os.environ.keys():
    print("METHOD_TOOLS_ROOT not defined in environement!")
    os.environ['METHOD_TOOLS_ROOT'] = '/tools/release'

if os.environ['SITE_'] == 'methodsm':
    sys.path.append( os.path.join("/usr/lib64/python2.5/site-packages"))
    sys.path.append( os.path.join("/usr/lib64/python2.5/site-packages/Ice"))
elif os.environ['SITE_'] == 'methodny':
    sys.path.append(os.path.join("/method/cg/opt/Linux_x86_64/lib/python2.5/site-packages"))

# defined above
#sys.path.append( os.path.join( "/usr/lib64/python2.5/site-packages" ) )
#sys.path.append( os.path.join( "/usr/lib64/python2.5/site-packages/Ice" ) )
sys.path.append( os.path.join( os.environ['METHOD_TOOLS_ROOT'], "lib/python/utils/" ) )
sys.path.append( os.path.join( os.environ['METHOD_TOOLS_ROOT'], "lib/python/setshot" ) )
sys.path.append( os.path.join( os.environ['METHOD_TOOLS_ROOT'], "lib/python/sendtoscratch" ) )
sys.path.append( os.path.join( os.environ['METHOD_TOOLS_ROOT'], "nuke/scripts/python" ) )

nuke.pluginAddPath( "./menu" )
nuke.pluginAddPath('./plugins')
nuke.pluginAddPath('./scripts/python/nukeStudio/gizmos')

# The folder /tools/apps/share/assets/resources/test_patterns
# holds a menu.py that will auto-add a menu of test patterns under
# Image for us. It contains both marcie and arri
nuke.pluginAddPath( "/tools/apps/share/assets/resources/test_patterns")

# vim:ts=4:sw=4:expandtab
# testing svn
# second addition
