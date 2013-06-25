#!python -u

import os, sys
import datetime
import re
import glob
import tarfile
import subprocess

def get_configuration_name(debug):
    configuration = 'WindowsVista'

    if debug:
        configuration += 'Debug'
    else:
        configuration += 'Release'

    return configuration

def get_configuration(debug):
    configuration = 'Windows Vista'

    if debug:
        configuration += ' Debug'
    else:
        configuration += ' Release'

    return configuration


def get_target_path(arch, debug):
    configuration = get_configuration_name(debug)

    target = { 'x86': 'proj', 'x64': os.sep.join(['proj', 'x64']) }
    target_path = os.sep.join([target[arch], configuration])

    return target_path


def shell(command):
    print(command)
    sys.stdout.flush()

    pipe = os.popen(command, 'r', 1)

    for line in pipe:
        print(line.rstrip())

    return pipe.close()


class msbuild_failure(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def msbuild(name, arch, debug):
    cwd = os.getcwd()
    configuration = get_configuration(debug)

    os.environ['SOLUTION'] = name

    if arch == 'x86':
        os.environ['PLATFORM'] = 'Win32'
    elif arch == 'x64':
        os.environ['PLATFORM'] = 'x64'

    os.environ['CONFIGURATION'] = configuration
    os.environ['TARGET'] = 'Build'

    os.chdir('proj')
    status = shell('msbuild.bat')
    os.chdir(cwd)

    if (status != None):
        raise msbuild_failure(configuration)


def callfnout(cmd):
    print(cmd)

    sub = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    output = sub.communicate()[0]
    ret = sub.returncode

    if ret != 0:
        raise(Exception("Error %d in : %s" % (ret, cmd)))
    print("------------------------------------------------------------")
    return output.decode('utf-8')

def archive(filename, files, tgz=False):
    access='w'
    if tgz:
        access='w:gz'
    tar = tarfile.open(filename, access)
    for name in files :
        try:
            print('adding '+name)
            tar.add(name)
        except:
            pass
    tar.close()


if __name__ == '__main__':
    os.environ['MAJOR_VERSION'] = '7'
    os.environ['MINOR_VERSION'] = '0'
    os.environ['MICRO_VERSION'] = '0'

    if 'BUILD_NUMBER' not in os.environ.keys():
        os.environ['BUILD_NUMBER'] = '0'

    if 'MERCURIAL_REVISION' in os.environ.keys():
        revision = open('revision', 'w')
        print(os.environ['MERCURIAL_REVISION'], file=revision)
        revision.close()

    debug = { 'checked': True, 'free': False }

    msbuild('xenpong', 'x86', debug[sys.argv[1]])
    msbuild('xenpong', 'x64', debug[sys.argv[1]])

    callfnout(['git', 'archive', '--format=tar.gz', '--prefix=source/', '-o', 'xenpong\\source.tgz', 'master'])
    archive('xenpong.tar', ['xenpong', 'revision'])
