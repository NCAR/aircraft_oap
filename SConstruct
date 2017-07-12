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
else:
    # Temporarily only build process2d if /opt/local install, until we get vardb submodule.
    SConscript('raf/process2d')

subdirs = ['cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'translate2ds', 'xpms2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))
