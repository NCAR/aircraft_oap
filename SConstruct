#!python

# This SConstruct orchestrates building OAP programs.

import os
import eol_scons

env = Environment(platform = 'posix',ENV= os.environ)

PREFIX='#'
try: env['JLOCAL'] = os.environ['JLOCAL']
except KeyError:
    PREFIX = '#'
else:
    PREFIX = env['JLOCAL']

Export('PREFIX')

subdirs = ['raf', 'cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'process2d', 'translate2ds', 'xpms2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))
