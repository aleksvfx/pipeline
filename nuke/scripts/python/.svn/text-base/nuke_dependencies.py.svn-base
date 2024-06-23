#!/usr/bin/python2.5
#
# Copyright 2010 brian leair
#
# Nuke python code to examine DAG for it's references
#
import nuke
import sys
import os

# Add in our facillity python tools
kMethodToolsRoot = "METHOD_TOOLS_ROOT"
gToolsRoot       = "/tools/release"
gMethPyPath      = os.path.join( gToolsRoot, "lib/python" )   
if kMethodToolsRoot in os.environ:
    gToolsRoot  = os.environ[kMethodToolsRoot]
    gMethPyPath = os.path.join( gToolsRoot, "lib/python" )
if (not gMethPyPath in sys.path):
    sys.path.append (gMethPyPath)

#import utils
#utils.addMethDirToPath ('lib/python') 
from facility import fs_utils

def listDependencies (output_path = None, output_only_dirs=False):
    # Will write text to the indicated fh (e.g. sys.stdout
    # or a fh you openend)

    # Complain if not saved..

    # We are assuming proxies live under the real
    # files, so get value of filename

    if (output_path):
        output_fh = open (output_path, "a")
    else:
        output_fh = sys.stdout

    # Set of paths (likely formatted paths)
    nodes = nuke.allNodes ()
    dependencies = set ()
    for node in nodes:
        knobs = node.knobs ()
        if ("disable" in knobs):
            if (node.knob ("disable").getValue ()):
                # Node isn't active, skip
                continue
        if ((node.Class () == "Read")
            or (node.Class () == "ReadGeo2")):
            # both read and readgeo2 have same knob
            dep = node.knob ("file").getValue ()

            real_dep = dep
            dep_dir, dep_filename = os.path.split (dep)
            if ("%" in dep):
                # translate through sym links (and do this to the
                # directory only because we have a formatted path
                if (os.path.exists (dep_dir)):
                    real_dep_dir = os.path.abspath (dep_dir)
                    real_dep = os.path.join (real_dep_dir, dep_filename)
            else:
                if (os.path.exists (dep)):
                    real_dep = os.path.abspath (dep)
            dependencies.add (real_dep)
        if (node.Class () == "Camera2"):
            # It's a bit trickier with cameras
            #if (node.knob ("read_from_file").getValue ()):
            #    # this camera is set to read from file
            dep = node.knob ("file").getValue ()
            if (dep):
                if (os.path.exists (dep)):
                    real_dep = os.path.abspath (dep)
                    dependencies.add (real_dep)

    # At this point, for those paths that are valid, we've
    # translated through symlinks.

    # Now warn if:
    # - any of the paths aren't inside of job
    # - any of the paths are to folders that don't exist
    for dep in dependencies:
        if ("%" in dep):
            dep_dir, dep_filename = os.path.split (dep)
            # translate through sym links (and do this to the
            # directory only because we have a formatted path)
            if (os.path.exists (dep_dir)):
                real_dep_dir = os.path.abspath (dep_dir)
                if (not os.path.exists (real_dep_dir)):
                    print >> sys.stderr, \
                        "WARN: nuke_dependencies: symlink %s points to invalid location %s." % (dep_dir, real_dep_dir)
            else:
                print >> sys.stderr, \
                    "WARN: nuke_dependencies: no directory %s." % (dep_dir)
        else:
            if (not os.path.exists (dep)):
                print >> sys.stderr, \
                    "WARN: nuke_dependencies: no file %s.", dep
        if (not isPathInJob (dep)):
            print >> sys.stderr, " "
            print >> sys.stderr, \
                "WARN: nuke_dependencies: !!!!!"
            print >> sys.stderr, \
                "WARN: nuke_dependencies: file %s is not stored under job." % (dep)
            print >> sys.stderr, \
                "WARN: nuke_dependencies: !!!!!"

    if (output_only_dirs):
        for dep in dependencies:
            dep_dir, dep_filename = os.path.split (dep)
            print >> output_fh, dep_dir
    else:
        for dep in dependencies:
            print >> output_fh, dep

    return

def isPathInJob (path):
    # getShotFromPath () is formal and strict about naming convetions
    # so it doesn't tolerate
    # /jobs/noes_noes09/common/images/textures/freddy_face_wound_matte
    # So, we can't use this method.
    # job,seq,shot = fs_utils.getShotFromPath (path)
    # Weaker check, just make sure file lives under /jobs/<job_name>
    job_path = os.environ ['JOB_PATH_']
    if (not path.startswith (job_path)):
        return False
    return True

