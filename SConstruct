#!python

# This SConstruct orchestrates building OAP programs.

import os
import eol_scons

def OAP_utils(env):
    env.Require(['prefixoptions'])
    env.Append(CPPPATH=[env['OPT_PREFIX']+'/vardb'])
    env.Append(LIBPATH=[env['OPT_PREFIX']+'/vardb'])
    env.Append(LIBPATH=[env['OPT_PREFIX']+'/vardb/raf'])

env = Environment(GLOBAL_TOOLS = [OAP_utils])

if env['INSTALL_PREFIX'] == '#':
    SConscript('vardb/SConscript')
    SConscript('vardb/raf/SConscript')


subdirs = ['cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'translate2ds', 'xpms2d', 'process2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))
