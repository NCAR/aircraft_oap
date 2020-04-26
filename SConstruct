#!python

# This SConstruct orchestrates building OAP programs.

import re
import os
import sys
import SCons
sys.path.append('vardb/site_scons')
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
    env['DEFAULT_INSTALL_PREFIX'] = GetOption('prefix')
    env['DEFAULT_OPT_PREFIX'] = GetOption('prefix')
    env.Require(['prefixoptions'])
 
env = Environment(GLOBAL_TOOLS = [OAP_utils])

def VARDB_opt(env):
    env.Append(CPPPATH=[env['OPT_PREFIX']+'/vardb'])
    env.Append(LIBS=['VarDB'])

try:
    env.Require('vardb')
except SCons.Errors.SConsEnvironmentError:
    print("local vardb source not found, building against installed vardb")
    vardb = VARDB_opt
    Export('vardb')

subdirs = ['cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'translate2ds', 'xpms2d', 'process2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))

variables = env.GlobalVariables()
variables.Update(env)
Help(variables.GenerateHelpText(env))
