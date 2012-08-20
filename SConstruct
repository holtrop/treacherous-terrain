# vim:filetype=python

import os

client_name = 'treacherous-terrain'
server_name = client_name + '-server'

# determine our build platform
platform = 'windows' if os.path.exists('/bin/cygwin1.dll') else 'unix'

# common environment settings
SFML_VERSION = '2.0-rc'
BIN_DIR = 'bin'
CXX = 'g++'
CXXFLAGS = ['-Wall', '-O2']
LINKFLAGS = []
LIBS_client = []
LIBS_server = []
libs_to_copy = []
SFML_PATH_PREFIX = '/c/apps' if platform == 'windows' else '/opt'
SFML_PATH = '%s/SFML-%s' % (SFML_PATH_PREFIX, SFML_VERSION)
if 'SFML_PATH' in os.environ:
    SFML_PATH = os.environ['SFML_PATH']
CPPFLAGS = ['-I%s/include' % SFML_PATH]
LIBPATH = ['%s/lib' % SFML_PATH]
for dirent in os.listdir('src'):
    CPPFLAGS.append('-I%s/src/%s' % (os.getcwd(), dirent))

if platform == 'windows':
    # Windows-specific environment settings
    CXX = 'i686-pc-mingw32-g++'
    MINGW_DIR = '/usr/i686-pc-mingw32/sys-root/mingw/bin'
    LIBS_client += ['sfml-graphics-s', 'sfml-window-s', 'sfml-system-s',
            'sfml-network-s', 'opengl32', 'mingw32']
    LIBS_server += ['sfml-network-s', 'mingw32']
    LINKFLAGS.append('-static-libstdc++')
    libs_to_copy.append('%s/libgcc_s_dw2-1.dll' % MINGW_DIR)
    CPPFLAGS.append('-DSFML_STATIC')
else:
    LIBS_client += ['sfml-network', 'sfml-window', 'sfml-graphics',
            'sfml-system', 'GL']
    LIBS_server += ['sfml-network']
    LINKFLAGS.append('-Wl,-R%s/lib' % SFML_PATH)

# our sources
sources_client = [Glob('src/common/*.cc'), Glob('src/client/*.cc')]
sources_server = [Glob('src/common/*.cc'), Glob('src/server/*.cc')]

# create the scons environments
env_client = Environment(
        CXX = CXX,
        CPPFLAGS = CPPFLAGS,
        CXXFLAGS = CXXFLAGS,
        LINKFLAGS = LINKFLAGS,
        LIBPATH = LIBPATH,
        LIBS = LIBS_client)
env_server = Environment(
        OBJSUFFIX = '-server.o',
        CXX = CXX,
        CPPFLAGS = CPPFLAGS,
        CXXFLAGS = CXXFLAGS,
        LINKFLAGS = LINKFLAGS,
        LIBPATH = LIBPATH,
        LIBS = LIBS_server)

for lib_path in libs_to_copy:
    installed_libs = env_client.Install(BIN_DIR, lib_path)
    env_client.Depends('%s/%s' % (BIN_DIR, client_name), installed_libs)
env_client.Program('%s/%s' % (BIN_DIR, client_name), sources_client)
env_server.Program('%s/%s' % (BIN_DIR, server_name), sources_server)
