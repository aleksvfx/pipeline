import sys
sys.path.append('..')

from simplecmd import SimpleCmd

helpText = '''
Nuke5.0 --help
Usage: nuke <switches> <script> <argv> <ranges>
  -a       formats default to anamorphic
  -b       background (no terminal, fork)
  -c size  limit cache memory usage. Size is in bytes, or append k, M or G
  -d name  set X display name
  -f       run full-size (turn off proxy)
  -g       gui only (no terminal)
  -help    print this help and exit
  -i       start up iconized (with -x or -t use interactive, not render, license)
  -l       apply linear transfer to the file read in
  -m n     set threads to n
  -n       don't run postagestamps, don't open windows
  -p       turn on proxy mode
  -q       quiet (don't print stuff)
  -s n     sets the minimum stack size for each thread in bytes, this defaults
           to 16777216 (16MB) the smallest allowed value is 1048576 (1MB)
  -t       terminal only (no gui)
  -V       verbosity (print more stuff)
  -v       nukev (rest of command line is image files to view)
  -view V  views to execute for (V is comma-separated list: e.g. 'left,right')
  -version print version information and exit
  -x       execute the script (rather than edit it)
  -X node  execute only this node
  --       end switches, allowing script to start with a dash
<script>:
  name of a .nuke script to create, edit, or execute
  "-" means stdin
<argv>:
  All words between the script name and the frame ranges can be used by
  [argv n] expressions to provide changing arguments to the script.
  Each must start with a non-digit to avoid confusion with frame ranges.
<ranges>:
  Frame numbers to execute the script at.
  A       single frame number A
  A,B     all frames from A through B
  A,B,C   every C'th frame from A to last one less or equal to B
'''

def create():
    # Create Nuke Render    
    cmdjob = SimpleCmd('Nuke (cmdline)', hasRange=True, canChunk=True, help='Nuke render jobtype')
    cmdjob.command = ('"%(executable)s" -t -x %(argv)s -- "%(script)s" %(scriptArgs)s QB_FRAME_START,QB_FRAME_END,QB_FRAME_STEP')

    cmdjob.add_optionGroup('Primary parameters')
    cmdjob.add_option( 'executable', 'file' , 'explicit path to Nuke executable', mode='open', default = 'Nuke5.0', choices=['Nuke5.0', 'C:/Program Files/Nuke5.0v1/nuke5.0.exe', '/Applications/Nuke5.0v1/Nuke5.0v1.app/Contents/MacOS/Nuke5.0', '/usr/local/Nuke5.0v1/Nuke5.0'])
    cmdjob.add_option( 'script'    , 'file' , 'nuke script to execute', mode='open', default = '',  mask='Nuke Script (*.nk)|*.nk|All files (*.*)|*.*')
    cmdjob.add_optionGroup('Optional Switches and Args')
    cmdjob.add_option( '-a', 'flag', 'formats default to anamorphic', label='anamorphic')
    cmdjob.add_option( '-b', 'flag', 'background (no terminal, fork)', label='background')
    cmdjob.add_option( '-c', 'string', 'limit cache memory usage. Size is in bytes, or append k, M or G', label='cache size')
    cmdjob.add_option( '-l', 'flag', 'apply linear transfer to the file read in', label='linear transfer')
    cmdjob.add_option( '-m', 'int', 'set threads to n', label='threads')
    cmdjob.add_option( '-p', 'flag', 'turn on proxy mode', label='proxy mode')
    cmdjob.add_option( '-q', 'flag', 'quiet (do not print stuff)', label='quiet')
    cmdjob.add_option( '-s', 'int', 'sets the minimum stack size for each thread in bytes, this defaults to 16777216 (16MB) the smallest allowed value is 1048576 (1MB)', label='stack size')
    cmdjob.add_option( '-V', 'flag', 'verbosity (print more stuff)', label='verbose')
    cmdjob.add_option( '-view', 'string', "views to execute for (comma-separated list: e.g. 'left,right')", label='view')
    cmdjob.add_option( '-X', 'string', 'execute only this node', label='execute node')
    cmdjob.add_option( 'scriptArgs', 'string', 'All words between the script name and the frame ranges can be used by [argv n] expressions to provide changing arguments to the script. Each must start with a non-digit to avoid confusion with frame ranges.', label='script args')
    # Omitted Properties (embedded in command directly or not applicable to rendering)
    #-d name  set X display name
    #-f       run full-size (turn off proxy)
    #-g       gui only (no terminal)
    #-help    print this help and exit
    #-i       start up iconized (with -x or -t use interactive, not render, license)
    #-n       don't run postagestamps, don't open windows
    #-t       terminal only (no gui)
    #-v       nukev (rest of command line is image files to view)
    #-x       execute the script (rather than edit it)
    #-version print version information and exit
    #--       end switches, allowing script to start with a dash
    #<ranges>:
    #Frame numbers to execute the script at.
    #A       single frame number A
    #A,B     all frames from A through B
    #A,B,C   every C'th frame from A to last one less or equal to B
    
    return [cmdjob]


if __name__ == '__main__':
    import wx
    import submit
    import logging
    import simplecmd
    logging.basicConfig(level=logging.DEBUG)
    app = submit.TestApp(redirect=False)
    cmds = create()
    for cmd in cmds:
        simplecmd.createSubmitDialog(cmd)
    app.MainLoop()
