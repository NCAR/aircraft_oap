#!python

# This SConstruct orchestrates building OAP programs.

import os
import sys
sys.path.append(os.path.abspath('vardb/site_scons'))
import eol_scons


def OAP_utils(env):
    if env['PLATFORM'] == 'darwin' and env['HOST_ARCH'] == 'arm64':
        env['DEFAULT_OPT_PREFIX']='/opt/homebrew'

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
   pub = env.Install('$PUBLISH_PREFIX', ["doc/OAPfiles.html"])
   env.Alias('publish', pub)

env.SetHelp()
env.AddHelp("""
Targets:
publish:  Copy html documentation to EOL web space : $PUBLISH_PREFIX.
""")
