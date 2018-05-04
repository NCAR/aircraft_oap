#!python

# This SConstruct orchestrates building OAP programs.

import re
import os

# Starting with version 2.3, scons searches for the following site_scons dirs in the 
# order shown (taken from eol_scons documentation):
#    /usr/share/scons/site_scons
#    $HOME/.scons/site_scons
#    ./site_scons
# so check for these before asking user to provide path on command line
if ((not os.path.exists("/usr/share/scons/site_scons")) and
    (not os.path.exists(os.environ['HOME']+"/.scons/site_scons")) and
    (not os.path.exists("./site_scons")) and
    (not re.match(r'(.*)site_scons(.*)',str(GetOption('site_dir'))))
   ):
    print "Must include --site-dir=vardb/site_scons (or whatever path to site_scons is) as a command line option. Exiting"
    Exit()

import eol_scons

AddOption('--prefix',
  dest='prefix',
  type='string',
  nargs=1,
  action='store',
  metavar='DIR',
  default='#',
  help='installation prefix')

def OAP_utils(env):
    if GetOption('prefix') != "#":
        env.Replace(DEFAULT_INSTALL_PREFIX = GetOption('prefix'))
    else:
        env['DEFAULT_INSTALL_PREFIX']="#"

    env['DEFAULT_OPT_PREFIX']="#"
    env.Require(['prefixoptions'])
 
    env.Append(CPPPATH=[env['OPT_PREFIX']+'/include'])
    env.Append(LIBPATH=[env['OPT_PREFIX']+'/lib'])

env = Environment(GLOBAL_TOOLS = [OAP_utils])

def VARDB_opt(env):
    env.Append(CPPPATH=[env['OPT_PREFIX']+'/vardb'])
    env.Append(LIBS=['VarDB'])

if env['INSTALL_PREFIX'] == '$DEFAULT_INSTALL_PREFIX':
    SConscript('vardb/SConscript')
else:
    vardb = VARDB_opt
    Export('vardb')

subdirs = ['cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'translate2ds', 'xpms2d', 'process2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))

variables = env.GlobalVariables()
variables.Update(env)
Help(variables.GenerateHelpText(env))
