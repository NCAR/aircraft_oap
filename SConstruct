#!python

# This SConstruct orchestrates building OAP programs.

import os
import sys
sys.path.append(os.path.abspath('vardb/site_scons'))
import eol_scons


def OAP_utils(env):
    env.Require(['prefixoptions'])
 
env = Environment(GLOBAL_TOOLS = [OAP_utils])

env.Require('vardb')

subdirs = ['2dsdump', 'cip/pads2oap', 'cip/padsinfo', 'oapinfo', 'translate2ds', 'xpms2d', 'process2d']

for subdir in subdirs:
    SConscript(os.path.join(subdir, 'SConscript'))

variables = env.GlobalVariables()
variables.Update(env)
Help(variables.GenerateHelpText(env))
