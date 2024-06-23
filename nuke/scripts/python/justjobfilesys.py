# ----------------------------------------------------------------------
# justjobfilesys.py
# (C) 2009 method 
# http://www.methodstudios.com 
# ----------------------------------------------------------------------
# Author - brian leair
"""
As of 8/24/09 the jobs filesystem was updated. From here on out
it is the "just job id filesystem". 
This update simplified paths but we may now need to update paths (like nuke).

Script will updates pathnames used by all read and write nodes 
What was
/jobs/noes/noes_noes09/sequences/NST/NST010/images/plates/NST010_plate_orig/2048x1556/NST010_plate_orig.0030
is now
/jobs/noes_noes09/sequences/nst/nst010/images/plates/nst010_plate_orig/2048x1556/nst010_plate_orig.0030
"""

import nuke
import os
import getpath

def updateScript ():
	path_converter = getpath.JustJobPathConverter ()
	if (not path_converter.haveJob ()):
		msg = "You don't curently have a job set. Please use ShotSetter."
		nuke.message (msg)
		return

	num_changed = 0
	num_checked = 0
	num_done = 0
	nodes = nuke.allNodes ()
	nodes_unchanged = []
	for node in nodes:
		if (node.Class () == "Read" or node.Class () == "Write"):
			num_checked += 1
			filename = node ['file'].value ()
			if (path_converter.willConvert (filename)):
				new_filename = path_converter.convert (filename)
				node['file'].setValue (new_filename) 
				orig_dir = os.path.dirname (filename)
				new_dir = os.path.dirname (new_filename)
				if ((os.path.isdir (orig_dir)) and 
					(not os.path.isdir (new_dir)) ):
					# Oh no.  old spot existed, but not new one
					msg = "Failed in the middle of updating '%s'\n" \
						"The old directory for\n\t%s\nexists, " \
						"howwever the expected new directory\n\t%s\nis missing. Please contact" \
						" software!" % (node.name (), filename, new_dir)
					nuke.message (msg)
					return
				num_changed += 1
			else:
				if (path_converter.isNewFormat (filename)):
					num_done += 1
				else:
					nodes_unchanged.append (node)


	msg_unchanged = "These non-job-filesystem nodes were left as-is:\n"
	for node in nodes_unchanged:
		msg_unchanged += "%s %s\n" % (node.name (), node['file'].value ())

	if (num_changed > 0):
		msg = "Updated %d of the %d read/write nodes (%d already in proper form)" % (
			num_changed, num_checked, num_done)
	else:
		msg = "Didn't need to change any of the %d read/write nodes.(%d already in proper form)" % (num_checked, num_done)
	if (num_checked == 0):
		msg = "No read or write nodes to update."

	if (len (nodes_unchanged) != 0):
		msg +=  "\n\n" + msg_unchanged
	nuke.message (msg)
	return
