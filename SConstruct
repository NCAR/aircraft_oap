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

env = Environment(PREFIX = GetOption('prefix'))

# Can't figure out how to Export the rhs, so assign it out to local variable.
PREFIX=env['PREFIX']

Export('PREFIX')

if env['PREFIX'] == '#':
    SConscript('raf/SConscript')
    SConscript('vardb/SConscript')

subdirs = ['cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'translate2ds', 'xpms2d', 'process2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))
