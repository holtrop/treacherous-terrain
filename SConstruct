# vim:filetype=python

import os
import sys
import re
from subprocess import Popen, PIPE

client_name = 'treacherous-terrain'
server_name = client_name + '-server'

CCFS_ROOT = 'assets/fs'

def find_dirs_under(path):
    dirs = [path]
    for ent in os.listdir(path):
        ent_path = '%s/%s' % (path, ent)
        if os.path.isdir(ent_path):
            dirs += find_dirs_under(ent_path)
    return dirs

def find_sources_under(path):
    sources = []
    for ent in os.listdir(path):
        ent_path = '%s/%s' % (path, ent)
        if re.search('\.cc?$', ent):
            sources.append(ent_path)
        elif os.path.isdir(ent_path):
            sources += find_sources_under(ent_path)
    return sources

# determine our build platform
platform = 'windows' if sys.platform == 'cygwin' else 'unix'

# common environment settings
SFML_VERSION = '2.0-rc'
BIN_DIR = 'bin'
CXX = 'g++'
CC = 'gcc'
CXXFLAGS = ['-Wall', '-O0', '-g']
LINKFLAGS = []
LIBS_client = []
LIBS_server = []
libs_to_copy = []
SFML_PATH_PREFIX = '/c/apps' if platform == 'windows' else '/opt'
SFML_PATH = '%s/SFML-%s' % (SFML_PATH_PREFIX, SFML_VERSION)
if 'SFML_PATH' in os.environ:
    SFML_PATH = os.environ['SFML_PATH']
LIBPATH = ['%s/lib' % SFML_PATH]
CPPFLAGS = []
CPPFLAGS += map(lambda x: '-I' + x, find_dirs_under('src/common'))
CPPFLAGS += ['-I%s/include' % SFML_PATH]
CPPFLAGS_client = ['-DGL_INCLUDE_FILE=\\"GL3/gl3w.h\\"']
CPPFLAGS_client += map(lambda x: '-I' + x, find_dirs_under('src/client'))
CPPFLAGS_server = map(lambda x: '-I' + x, find_dirs_under('src/server'))

if platform == 'windows':
    # Windows-specific environment settings
    CXX = 'i686-pc-mingw32-g++'
    CC = 'i686-pc-mingw32-gcc'
    MINGW_DIR = '/usr/i686-pc-mingw32/sys-root/mingw/bin'
    LIBS_client += ['sfml-graphics-s', 'sfml-window-s', 'sfml-network-s',
            'sfml-system-s', 'opengl32', 'glu32', 'mingw32']
    LIBS_server += ['sfml-network-s', 'sfml-system-s', 'mingw32']
    LINKFLAGS.append('-static-libstdc++')
    libs_to_copy.append('%s/libgcc_s_dw2-1.dll' % MINGW_DIR)
    CPPFLAGS.append('-DSFML_STATIC')
else:
    LIBS_client += ['sfml-network', 'sfml-window', 'sfml-graphics',
            'sfml-system', 'GL', 'GLU']
    LIBS_server += ['sfml-system','sfml-network']
    LINKFLAGS.append('-Wl,-R%s/lib' % SFML_PATH)

# our sources
sources_client = (find_sources_under('src/common') +
        find_sources_under('src/client'))
if 'src/client/ccfs.cc' not in sources_client:
    sources_client.append('src/client/ccfs.cc')
sources_server = (find_sources_under('src/common') +
        find_sources_under('src/server'))

# create the scons environments
env_client = Environment(
        CC = CC,
        CXX = CXX,
        CPPFLAGS = CPPFLAGS + CPPFLAGS_client,
        CXXFLAGS = CXXFLAGS,
        LINKFLAGS = LINKFLAGS,
        LIBPATH = LIBPATH,
        LIBS = LIBS_client)
env_server = Environment(
        OBJSUFFIX = '-server.o',
        CC = CC,
        CXX = CXX,
        CPPFLAGS = CPPFLAGS + CPPFLAGS_server,
        CXXFLAGS = CXXFLAGS,
        LINKFLAGS = LINKFLAGS,
        LIBPATH = LIBPATH,
        LIBS = LIBS_server)

# CCFS builder

def get_all_files(prefix):
    files = []
    for ent in os.listdir(prefix):
        if ent.startswith('.'):
            next
        full_path = '%s/%s' % (prefix, ent)
        if os.path.isdir(full_path):
            files += get_all_files(full_path)
        else:
            files.append(full_path)
    return files

def CCFS(target, source, env):
    source_list = []
    for s in source:
        source_fname = str(s)
        source_fname = source_fname.replace(CCFS_ROOT + '/', '')
        source_list.append(source_fname)
    Popen(['./ccfs_gen.py', '--root', 'assets/fs', str(target[0])] + source_list).wait()
    return None

def CCFS_emitter(target, source, env):
    target.append(re.sub(r'\..*$', '.h', str(target[0])))
    return target, source

env_client.Append(BUILDERS = {'CCFS' : Builder(action = CCFS, emitter = CCFS_emitter)})

env_client.CCFS('src/client/ccfs.cc', get_all_files(CCFS_ROOT))
env_client.Depends('src/client/ccfs.cc', 'ccfs_gen.py')

for lib_path in libs_to_copy:
    installed_libs = env_client.Install(BIN_DIR, lib_path)
    env_client.Depends('%s/%s' % (BIN_DIR, client_name), installed_libs)
env_client.Program('%s/%s' % (BIN_DIR, client_name), sources_client)
env_server.Program('%s/%s' % (BIN_DIR, server_name), sources_server)
