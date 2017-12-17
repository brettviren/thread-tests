#!/usr/bin/env fab

from fabric.api import run, env, settings, cd

# hosts on which to run tests.  In general, depends on .ssh/config
# entries for access.
#env.hosts = ["hal","haiku","hierocles.bnl","hothstor2.bnl"]
#env.hosts = ["hal","haiku"]
env.use_ssh_config = True

def gather_info():
    host = env['host']
    cpu = run("grep '^model name' /proc/cpuinfo  |head -1")
    ncores = run("grep -c '^processor' /proc/cpuinfo")
    print host,cpu,ncores
    

def doitall():
    giturl="https://github.com/brettviren/thread-tests.git"
    tmpdir = run('mktemp -d /tmp/thread-tests-XXXXX')
    srcdir = "%s/src" % (tmpdir, )

    i1 = run("grep '^model name' /proc/cpuinfo  |head -1")
    i2 = run("grep -c '^processor' /proc/cpuinfo")
    i3 = run("free --giga")

    run("git clone -q %s %s" % (giturl,srcdir))
    
    with cd(srcdir):
        run("./waf -p configure build")
        t1 = run("./build/test_modulo")
        t2 = run("./build/test_arene")
        t3 = run("./uds.sh")
 
    host = env['host']
    with open("%s.log" % host, "w") as fp:
        fp.write('host:%s\n%s\n%s\n%s\n%s\nncores: %s\n%s\n' % \
                 (host,t1,t2,t3,i1,i2,i3))

    run("rm -rf %s" % tmpdir)

def uds():
    run ('nohup socat -u -b32768 UNIX-LISTEN:/tmp/unix.sock /dev/null >/dev/null 2&1 &')
    run ('socat -u -b32768 "SYSTEM:dd if=/dev/zero bs=1M count=1024" UNIX:/tmp/unix.sock')
