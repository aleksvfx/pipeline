import sys
import nuke

print 'Starting Nuke @ Smoke & Mirrors Amsterdam'
nuke.pluginAddPath('Y:/config/nuke/gizmos/external/')
nuke.pluginAddPath('Y:/config/nuke/gizmos/external/Grain_CB')
nuke.pluginAddPath('Y:/config/nuke/gizmos/external/HeatWave 3.0')
nuke.pluginAddPath('Y:/config/nuke/gizmos/external/mmColorTarget_v2.0')

nuke.pluginAddPath('Y:/config/nuke/plugins/')
nuke.pluginAddPath('Y:/config/nuke/plugins/CreatePath')

nuke.pluginAddPath('Y:/config/nuke/icons')
nuke.pluginAddPath('Y:/config/nuke/scripts')

nuke.pluginAddPath('Y:/config/nuke/modules')
nuke.pluginAddPath('Y:/config/nuke/modules/numpy')

nuke.pluginAddPath('Y:/config/nuke/plugins/windows/Nuke10.0')

import numpy
import collectFiles
import makewritefromread
