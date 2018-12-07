import sys
import nuke

def setViewerProcess():
	nuke.thisNode().knob('viewerProcess').setValue('rec709')

nuke.addOnUserCreate(setViewerProcess,nodeClass = 'Viewer')

menubar = nuke.menu("Nuke")
toolbar = nuke.toolbar("Nodes")


# If Write dir does not exist, create it
def createWriteDir():
    file = nuke.filename(nuke.thisNode())
    dir = os.path.dirname( file )
    osdir = nuke.callbacks.filenameFilter( dir )
    try:
        os.makedirs( osdir )
        return
    except:
        return

# Activate the createWriteDir function
nuke.addBeforeRender(createWriteDir)

# Make Read & Write node default to rec709 color space
nuke.knobDefault('Read.mov.colorspace', 'rec709')
nuke.knobDefault('Write.mov.colorspace', 'rec709')

smoke_gizmos = toolbar.addMenu("Smoke gizmos", icon="sm.png")
smoke_gizmos.addCommand("Grain_CB", "nuke.createNode('Grain_CB.gizmo')", icon="GrainCB.png")
smoke_gizmos.addCommand("BokehBlur", "nuke.createNode('BokehBlur.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("L_Fuse", "nuke.createNode('L_Fuse_v06.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("LCA_Zdepth", "nuke.createNode('LCA_Zdepth.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("ChromAbb", "nuke.createNode('ChromAbb.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("DespillMadness", "nuke.createNode('DespillMadness.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("EdgeExtend2", "nuke.createNode('EdgeExtend2.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("OpticalZDefocus", "nuke.createNode('OpticalZDefocus.nk')", icon="sm.png")
smoke_gizmos.addCommand("tpInsta_FX", "nuke.createNode('tpInsta_FX.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("alexaFilmMatrix", "nuke.createNode('alexaFilmMatrix.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("FireflyKiller", "nuke.createNode('FireflyKiller.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("Firefly_Swatter", "nuke.createNode('Firefly_Swatter.gizmo')", icon="sm.png")
smoke_gizmos.addCommand("mmColorTarget", "nuke.createNode('mmColorTarget.gizmo')", icon="mmColorTarget.png")

smoke_tools = toolbar.addMenu("Smoke tools", icon="sm.png")
smoke_tools.addCommand("Make Write of Read...", 'makewritefromread.make_write_from_read()', '')
smoke_tools.addCommand("nFX/CreatePath", 'nuke.load("CreatePath.py")', '')

collectMenu = toolbar.addMenu("Collect_Files")
collectMenu.addCommand('Collect Files', 'collectFiles.collectFiles()')
collectMenu.addCommand('Help', 'collectFiles.myBlog()')

nuke.menu("Nodes").addCommand("3DE4/LD_3DE4_Anamorphic_Standard_Degree_4", "nuke.createNode('LD_3DE4_Anamorphic_Standard_Degree_4')")
nuke.menu("Nodes").addCommand("3DE4/LD_3DE4_Anamorphic_Rescaled_Degree_4", "nuke.createNode('LD_3DE4_Anamorphic_Rescaled_Degree_4')")
nuke.menu("Nodes").addCommand("3DE4/LD_3DE4_Anamorphic_Degree_6", "nuke.createNode('LD_3DE4_Anamorphic_Degree_6')")
nuke.menu("Nodes").addCommand("3DE4/LD_3DE4_Radial_Standard_Degree_4", "nuke.createNode('LD_3DE4_Radial_Standard_Degree_4')")
nuke.menu("Nodes").addCommand("3DE4/LD_3DE4_Radial_Fisheye_Degree_8", "nuke.createNode('LD_3DE4_Radial_Fisheye_Degree_8')")
nuke.menu("Nodes").addCommand("3DE4/LD_3DE_Classic_LD_Model", "nuke.createNode('LD_3DE_Classic_LD_Model')")
