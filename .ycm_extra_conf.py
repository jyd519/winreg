import os.path as p
import platform
import subprocess
import sys


DIR_OF_THIS_SCRIPT = p.abspath(p.dirname(__file__))
SOURCE_EXTENSIONS = ['.cpp', '.cxx', '.cc', '.c', '.m', '.mm']

DIR_OF_HOME = p.expanduser('~')

# These are the compilation flags that will be used in case there's no
# compilation database set (by default, one is not set).
# CHANGE THIS LIST OF FLAGS. YES, THIS IS THE DROID YOU HAVE BEEN LOOKING FOR.
if platform.system() == 'Windows':
    flags = [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-Wno-long-long',
        '-Wno-variadic-macros',
        '-fexceptions',
        '-DNDEBUG',
        '-DUNICODE',
        '-D_UNICODE',
        '-DWIN32',
        '-D_WIN32',
        '-x', 'c++',
        '-isystem', 'C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\atlmfc\\include',
        '-isystem', r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\include',
        '-isystem', r'C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\crt\src\concrt',
        '-isystem', r'C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\ucrt',
        '-I' + p.join(DIR_OF_HOME, '.node-gyp\\12.13.1\\include\\node'),
        '-I', p.join(DIR_OF_THIS_SCRIPT, './node_modules/node-addon-api/'),
        '-isystem', r'C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\um',
        '-isystem', r'C:\Program Files (x86)\Windows Kits\10\Include\10.0.17763.0\shared',
    ]
else:
    flags = [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-Wno-long-long',
        '-Wno-variadic-macros',
        '-fexceptions',
        '-DNDEBUG',
        '-x', 'c++',
        '-I', '/usr/local/include',
        '-I', p.join(DIR_OF_HOME, '.nvm/versions/node/v12.0.0/include/node'),
        '-I', p.join(DIR_OF_THIS_SCRIPT, './node_modules/node-addon-api/'),
    ]

# Clang automatically sets the '-std=' flag to 'c++14' for MSVC 2015 or later,
# which is required for compiling the standard library, and to 'c++11' for older
# versions.
if platform.system() != 'Windows':
    flags.append('-std=c++14')


# Set this to the absolute path to the folder (NOT the file!) containing the
# compile_commands.json file to use that instead of 'flags'. See here for
# more details: http://clang.llvm.org/docs/JSONCompilationDatabase.html
#
# You can get CMake to generate this file for you by adding:
#   set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
# to your CMakeLists.txt file.
#
# Most projects will NOT need to set this to anything; you can just change the
# 'flags' list of compilation flags. Notice that YCM itself uses that approach.
compilation_database_folder = ''
database = None
# if p.exists(compilation_database_folder):
#     import ycm_core
#     database = ycm_core.CompilationDatabase(compilation_database_folder)
# else:
#     database = None


def IsHeaderFile(filename):
    extension = p.splitext(filename)[1]
    return extension in ['.h', '.hxx', '.hpp', '.hh']


def FindCorrespondingSourceFile(filename):
    if IsHeaderFile(filename):
        basename = p.splitext(filename)[0]
        for extension in SOURCE_EXTENSIONS:
            replacement_file = basename + extension
            if p.exists(replacement_file):
                return replacement_file
    return filename


def Settings(**kwargs):
    language = kwargs['language']

    if language == 'cfamily':
        # If the file is a header, try to find the corresponding source file and
        # retrieve its flags from the compilation database if using one. This is
        # necessary since compilation databases don't have entries for header files.
        # In addition, use this source file as the translation unit. This makes it
        # possible to jump from a declaration in the header file to its definition
        # in the corresponding source file.
        filename = FindCorrespondingSourceFile(kwargs['filename'])

        if filename.endswith('.mm'):
            for i, flag in enumerate(flags):
                if flag == '-x':
                    flags[i+1] = 'objective-c++'

        if not database:
            return {
                'flags': flags,
                'include_paths_relative_to_dir': DIR_OF_THIS_SCRIPT,
                'override_filename': filename
            }

        compilation_info = database.GetCompilationInfoForFile(filename)
        if not compilation_info.compiler_flags_:
            return {}

        # Bear in mind that compilation_info.compiler_flags_ does NOT return a
        # python list, but a "list-like" StringVec object.
        final_flags = list(compilation_info.compiler_flags_)

        # NOTE: This is just for YouCompleteMe; it's highly likely that your project
        # does NOT need to remove the stdlib flag. DO NOT USE THIS IN YOUR
        # ycm_extra_conf IF YOU'RE NOT 100% SURE YOU NEED IT.
        try:
            final_flags.remove('-stdlib=libc++')
        except ValueError:
            pass

        return {
            'flags': final_flags,
            'include_paths_relative_to_dir': compilation_info.compiler_working_dir_,
            'override_filename': filename
        }

    return {}


def main():
    print(' '.join(Settings(filename=sys.argv[1], language='cfamily').get('flags')))

if __name__ == '__main__':
    sys.exit(main())
