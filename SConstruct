#!python

# This SConstruct orchestrates building OAP programs.

import os
import sys
# Put the bundled eol_scons first, so it wins over any personal copy in
# ~/.scons/site_scons.  Packagers (e.g. MacPorts) only have the bundled one
# so this ensures local tests reproduce what packagers see.
sys.path.insert(0, os.path.abspath('vardb/site_scons'))
import eol_scons


def OAP_utils(env):
    env.Require(['prefixoptions'])
 
env = Environment(GLOBAL_TOOLS = [OAP_utils])
env['PUBLISH_PREFIX'] = '/net/www/docs/raf/Software'


env.Require('vardb')

subdirs = ['2dsdump', 'cip/pads2oap', 'cip/padsinfo', 'extract2ds', 'oapinfo', 'xpms2d', 'process2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))

variables = env.GlobalVariables()
variables.Update(env)
Help(variables.GenerateHelpText(env))

if "xpms2d" in COMMAND_LINE_TARGETS:
   subdirs = ['xpms2d']

if "publish" in COMMAND_LINE_TARGETS:
   pub = env.Install('$PUBLISH_PREFIX', ["doc/OAPfiles.html","doc/pms2d_proc.html"])
   env.Alias('publish', pub)

env.SetHelp()
env.AddHelp("""
Targets:
publish:  Copy html documentation to EOL web space : $PUBLISH_PREFIX.
""")
