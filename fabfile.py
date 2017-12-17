#!/usr/bin/env fab

from fabric.api import run, env

# hosts on which to run tests.  In general, depends on .ssh/config
# entries for access.
#env.hosts = ["hal","haiku","hierocles.bnl","hothstor2.bnl"]
env.hosts = ["hal","haiku"]
env.use_ssh_config = True

def gather_info():
    host = env['host']
    cpu = run("grep '^model name' /proc/cpuinfo  |head -1")
    ncores = run("grep -c '^processor' /proc/cpuinfo")
    print host,cpu,ncores
    

def install():
    srcdir="/tmp/thread-tests"
    giturl="https://github.com/brettviren/thread-tests.git"
    if run("test -d %s" % srcdir).failed:
        run("git clone %s %s" % (giturl, srcdir))
    with cd(srcdir):
        run("./waf configure build")
