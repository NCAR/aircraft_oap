#!python

# This SConstruct orchestrates building OAP programs.

import os
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
