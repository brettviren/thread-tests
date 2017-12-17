def options(opt):
    opt.load('boost', tooldir="waftools")
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('boost', tooldir="waftools")
    conf.load('compiler_cxx')
    conf.env.CXXFLAGS += ["-O3", "-std=c++14"]
    conf.check_boost(lib='timer system')


def build(bld):
    bld.program(source='test_arene.cxx', 
                target='test_arene',
                includes=".",
                lib = ["boost_thread", "boost_system", "pthread"],
                use='BOOST')    
    bld.program(source='ringbuftest.cpp', 
                target='ringbuftest',
                includes=".",
                lib = ["boost_thread", "boost_system", "pthread"],
                use='BOOST')    
    bld.program(source='test_RingBufferSPSCLockFree.cxx', 
                target='test_modulo',
                includes=".",
                lib = ["boost_thread", "boost_system", "pthread"],
                use='BOOST')    
