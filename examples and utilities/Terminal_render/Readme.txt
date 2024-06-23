Add the following to the bottom of toolbars.py



m = toolbar.addMenu("LastpixelTools", "LP.png")
m.addCommand("MaxTester", "nuke.createNode(\"MaxTester\")")
m.addCommand("LaunchNuker", "nuke.load(\"lanchNukes_inNuke\"), launch_nukes()")


copy lanchnukes_inNuke.py and MaxTester.gizmo to your plug-ins directory
then copy the LP.png icon to your icons directory.

