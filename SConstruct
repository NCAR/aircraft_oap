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

env = Environment(PREFIX = GetOption('prefix'),ENV= os.environ)
#env = Environment(platform = 'posix',ENV= os.environ)

# Can't figure out how to Export the rhs, so assign it out to local variable.
PREFIX=env['PREFIX']

Export('PREFIX')

if env['PREFIX'] == '#':
    SConscript('raf/SConscript')

subdirs = ['cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'process2d', 'translate2ds', 'xpms2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))
