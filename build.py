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
    os.environ['CONFIGURATION'] = configuration

    set_platform(arch)

    os.chdir('proj')
    status = shell('msbuild.bat')
    os.chdir(cwd)

    if (status != None):
        raise msbuild_failure(configuration)

def set_platform(arch):
    if arch == 'x86':
        os.environ['PLATFORM'] = 'Win32'
    elif arch == 'x64':
        os.environ['PLATFORM'] = 'x64'

def callfnout(cmd):
    print(cmd)

    sub = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    output = sub.communicate()[0]
    ret = sub.returncode

    if ret != 0:
        raise(Exception("Error %d in : %s" % (ret, cmd)))
    print("------------------------------------------------------------")
    return output.decode('utf-8')

def main(debug):

    set_version()
    set_build_number()
    set_build_variable("xenpong")

    if 'MERCURIAL_REVISION' in os.environ.keys():
        revision = open('revision', 'w')
        print(os.environ['MERCURIAL_REVISION'], file=revision)
        revision.close()

    msbuild('xenpong', 'x86', debug)
    msbuild('xenpong', 'x64', debug)

    archive_source()
    archive('xenpong.tar', ['xenpong', 'revision'])

def archive_source():
    cmd = ['git', 'archive', '--format=tar.gz', '--prefix=source/', '-o', 'xenpong\\source.tgz', 'master']
    callfnout(cmd)

def archive(filename, files, tgz=False):
    access='w'
    if tgz:
        access='w:gz'
    with tarfile.open(filename, access) as tar:
        for name in files:
            try:
                print('adding ' + name)
                tar.add(name)
            except:
                pass

def set_version():
    os.environ['MAJOR_VERSION'] = '7'
    os.environ['MINOR_VERSION'] = '0'
    os.environ['MICRO_VERSION'] = '0'

def set_build_number():
    if 'BUILD_NUMBER' not in os.environ.keys():
        os.environ['BUILD_NUMBER'] = '0'

def set_build_variable(name):
    os.environ['TARGET'] = 'Build'
    os.environ['SOLUTION'] = name

def print_usage_and_exit():
    print ("Usage: %s <checked|free>" % (sys.argv[0], ))
    sys.exit(1)

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print_usage_and_exit()

    if sys.argv[1] == "checked":
        debug = True
    elif sys.argv[1] == "free":
        debug = False
    else:
        print_usage_and_exit()

    main(debug)
