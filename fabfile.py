#!/usr/bin/env fab

import json

from fabric.api import local, run, env, settings, cd

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
    

nelements=100000000
nbits=18
width=2560
latencies=[0, 1, 2, 4, 10, 100, 1000, 10000]

def junk():
    with cd("/home/bv/dev/mmaptest"):
        out = run("./build/test_modulo 10000")
    print 'OUT:', type(out)
    print str(out)
    print 'STDOUT:', type(out.stdout)
    print str(out.stdout)
    print 'STDERR:', type(out.stderr)
    print str(out.stderr)

def doitall():
    giturl="https://github.com/brettviren/thread-tests.git"
    tmpdir = run('mktemp -d /tmp/thread-tests-XXXXX')
    srcdir = "%s/src" % (tmpdir, )

    mem = run("free").split('\n')[1:]
    info = dict(
        cpu = run("grep '^model name' /proc/cpuinfo  |head -1").split(':',1)[1],
        ncpus = run("grep -c '^processor' /proc/cpuinfo"),
        host = env['host'],
        memory = mem[0].split()[1],
        swap = mem[1].split()[1]
    )

    run("git clone -q %s %s" % (giturl,srcdir))
    
    results = dict()
    with cd(srcdir):
        run("./waf -p configure build")
        jtext = run("./build/test_modulo %d 2>&1" % nelements).stdout
        results["test_modulo"] = json.loads(jtext)

        t2s = list()
        for latency in latencies:
            jtext = run("./build/test_arene %d %d %d %d" % (nelements, nbits, width, latency)).stdout
            t2s.append(json.loads(jtext))
        results["test_arene"] = t2s

        uds = run("./uds.sh").split('\n')
        uds1 = uds[2].split()
        uds2 = uds[5].split()
        results["uds"] = [
            dict(records=uds[0].split('+',1)[0],
                 nbytes=uds1[0],
                 time=uds1[6]),
            dict(records=uds[3].split('+',1)[0],
                 nbytes=uds2[0],
                 time=uds2[6])
        ]
 
    dat = dict(info=info, results=results)
    with open("{host}.json".format(**info), "w") as fp:
        fp.write(json.dumps(dat,indent=4))

    run("rm -rf %s" % tmpdir)

def uds():
    run ('nohup socat -u -b32768 UNIX-LISTEN:/tmp/unix.sock /dev/null >/dev/null 2&1 &')
    run ('socat -u -b32768 "SYSTEM:dd if=/dev/zero bs=1M count=1024" UNIX:/tmp/unix.sock')

