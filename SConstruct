#
# SConstruct for TvGuideFetch.
#

# Builds specified by mode=debug or mode=release on the command line
buildMode = ARGUMENTS.get('mode', 'release')
if not (buildMode in ['debug', 'release']):
	print "Error: expect mode=debug or mode=release, found: " + buildMode
	Exit(1)

# Configure common items in the environment
env = Environment()
env.Append(CPPDEFINES = '_GNU_SOURCE')
env.Append(CCFLAGS = ['-Wall'])

# Taylor the environment for a release build
if buildMode == 'release':
	env.Append(CCFLAGS = ['-Os'])

# Taylor the environment for a debug build
if buildMode == 'debug':
	env.Append(CPPDEFINES = 'DEBUG')
	env.Append(CCFLAGS = ['-Og', '-g'])

bldDir = 'build/' + buildMode + '/'
SConscript('SConscript', exports = 'env bldDir')
Clean('build', bldDir)
Clean('.', 'build')

# Install rule
env.Install('/usr/bin', bldDir + 'tv_grab_au_tvguide')
env.Alias('install', '/usr/bin')
