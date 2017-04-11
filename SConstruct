# -*- python -*-

import os
env = Environment(platform = 'posix',ENV= os.environ)
env['JLOCAL'] = os.environ['JLOCAL']

env.Append(CXXFLAGS='-g -Werror')
env.Append(CPPPATH="$JLOCAL/include")


SConscript('./cip/pads2oap/SConscript')
SConscript('./cip/padsinfo/SConscript')
SConscript('./oapinfo/SConscript')
SConscript('./process2d/SConscript')
SConscript('./translate2ds/SConscript')
SConscript('./xpms2d/SConscript')
