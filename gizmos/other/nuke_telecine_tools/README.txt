************* TELECINE TOOLS FOR NUKE *************
** CREATED 03/29/08 BY MICHAEL@YAWPITCHROLL.COM ***

Telecine.gizmo - Performs 2:3 Pullup 24p->30i
InvTelecine.gizmo - Performs 2:3 Pulldown 30i->24p

These files should give accurate stepping performance and have been tested for interoperability with the Shake filein node, whose 24 to 30 and 30 to 24 functions they are designed to replicate.

Both files are an improvement over the default Add32p and Remove32p nodes, which are somewhat confusing, mathematically incorrect (rounding errors contribute to occasional slips in interlacing cadence in longer than average sequences), and don't offset the in and out to compensate for the phase shift when selecting different cadence phases.

To compare and contrast these tools and the default ones, place a Read to load some 24 frame footage, then append first a Telecine and then an InvTelecine node, both with the same phase and fields settings.  Finally Merge back in the original Read with a Minus operation.  On a separate branch do the same operations with an Add32p/Remove32p combo, then do a viewer split to compare the outputs of the two Minus nodes.

While there can be errors at first and last frame due to the telecine operation trying to interlace from frames outside your footage start and stops frames, and ideal Telecine/InvTelecine operation will result in all black (000 in RGB) frames and no registration slippage when compared using the Minus operations described above.

************ INSTALLATION INSTRUCTIONS ************

The tools were created in Nuke 5.0 but have been tested in 4.8 and appear to work fine.  I've removed the flag that caused a warning message to pop up in 4.8, but it may still appear in earlier versions of 4.X, however you should have no problem using the plugins as all of the nodes in use are cross-compatible.

NUKE 5.X
1) Copy the .gizmo files to [nuke install directory]/plugins/user
2) Navigate to [nuke install directory]/[Python frameworks path]/site-packages/nukescripts/
3) Edit your toolbars.py file and add the following to the TIME MENU (HINT: look for 'toolbar.addMenu("Time", ToolbarTime.png")' and place the following after the 'm.addCommand("TimeWarp...' string)

m.addCommand("Telecine", "nuke.createNode(\"Telecine\")", icon="Add32.png")m.addCommand("Inverse Telecine", "nuke.createNode(\"InvTelecine\")", icon="Remove32.png")

4) Restart Nuke, the plugins will now be found in your Node Toolbard under the Clock (Time) icon.  Additionally you can use the Tab key in the Node Layout portion of the DAG to bring up the Quick Selector and simply start typing Telecine or InvTelecine to select and add the nodes.
5) Enjoy the wonderful world and nail-biting excitement of telecine and inverse telecine.

NUKE 4.X
1) Copy the .gizmo files to [nuke install directory]/plugins/user
2) Edit your menu.tcl file and add the following to the TIME MENU:

menu "&Time/Telecine" Telecine
menu "&Time/Inverse Telecine" InvTelecine

3) Restart Nuke, the plugins will now be found in your Time menu.
4) Enjoy the wonderful world and nail-biting excitement of telecine and inverse telecine.

**************** LICENSE DETAILS ******************
Released as freeware on fxshare.com, but if you feel like offering me a job I wouldn't complain.

***************** MISC EPHEMERA *******************
You've probably been hearing a lot about 3:2 Pulldown since you started in this business, but likely never understood it.  Good, no one does, and the few who do should probably have been stuffed and mounted a long time ago.  Not to say they're relics, just engineers of a certain age: the process is giddyingly stupefying in both its simplicity and its many, many ways of confusing people beyond the point of mere senility.  I have found that paying attention to details helps, and the biggest one is to correct, very clearly and concisely, anyone who actually describes it as "adding 3:2 Pulldown".  Correctly, it is 2:3 Pullup (aka Telecine) (when going from 24 to 30) and 2:3 Pulldown (aka Inverse or Reverse Telecine, or the much-more aggravating Remove Pulldown).  It's a bit nitpicky to harp on the Pullup vs Pulldown (although a good mnemonic is to think that since film is natural and video artificial, you Pull Up your trousers when you want to go out into the artificial world, and you Pull them Down when you want to be natural again), but the 2 MUST come before the 3, never the 3 before the 2, as otherwise you get the rhythm all wrong.  So, in the interests of making this all clearer, make sure you say 2 before 3, keep your trousers on when you're around video engineers, don't ever ask what the difference between 30 and 29.976 is for, and ignore anyone who's enough of an academic to take affront to that little stance I took there in the war between nature and artificiality.

********************* ALSO ************************
Yosarian lives, Pratchett's the funniest man alive, Gabriel Garcia Marquez is the best dirty old man these last hundred years, and somewhere in Texas there's a village missing its idiot, but that's ok because everybody gets emotional sometimes and, besides, they know where to find him and it won't be long now.

********************* EULA ************************
Anyone using this software will be attempting to make 24 = 30, or 30 = 24, and either way that's right on the edge of criminality, if you ask me, so basically they should probably sick homeland security on your behind just for downloading this thing, cause messing with constants is just downright un-American.  Either way, by installing and using this you're giving me unrestricted title to everything you own, women and children first, a measure of wheat for a penny, and three measures of barley for a penny; and see thou hurt not the oil and the wine.  But since I'm happily married you can keep the women ... come to think of it the children should probably stay with their mothers ... doctor says I should cut down on the carbs and I don't drink anyway, plus I really don't have room in this tiny apartment for all your stuff.  Hmm, ok, let's say you owe me a nice bottle of olive oil if you ever feel like it, plus maybe that job offer I mentioned up there somewhere.