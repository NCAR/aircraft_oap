#!python

import os
import eol_scons

env = Environment(platform = 'posix',ENV= os.environ)

try: env['JLOCAL'] = os.environ['JLOCAL']
except KeyError:
    env.Append(CPPPATH=['#/raf'])
    env.Append(LIBPATH=['#/raf'])
    env['BINDIR'] = '#/bin'
else:
    env.Append(CPPPATH=['$JLOCAL/include'])
    env.Append(LIBPATH=['$JLOCAL/lib'])
    env['BINDIR'] = '$JLOCAL/bin'


env.Append(CXXFLAGS='-g -Wall')

Export('env')

SConscript('./raf/SConscript')
SConscript('./cip/pads2oap/SConscript')
SConscript('./cip/padsinfo/SConscript')
SConscript('./oapinfo/SConscript')
SConscript('./process2d/SConscript')
SConscript('./translate2ds/SConscript')
SConscript('./xpms2d/SConscript')
