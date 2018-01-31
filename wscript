def options(opt):
    opt.load('boost', tooldir="waftools")
    opt.load('compiler_cxx')

def configure(conf):
    conf.load('boost', tooldir="waftools")
    conf.load('compiler_cxx')
    #conf.env.CXXFLAGS += ["-O3", "-std=c++14"]
    conf.env.CXXFLAGS += ["-g", "-std=c++14"]
    conf.check_boost(lib='timer system')
    conf.check_cfg(package='eigen3',  uselib_store='EIGEN', args='--cflags --libs')


def build(bld):
    bld.program(source='bigarray.cpp', 
                target='bigarray',
                includes=".",
                #lib = ["boost_thread", "boost_system", "pthread"],
                #use='BOOST'
    )    
    bld.program(source='test_nparray.cpp cnpy.cxx', 
                target='test_nparray',
                includes=".",
                lib = ["z"],
                use='EIGEN'
    )    
    bld.program(source='test_cnpy.cpp cnpy.cxx', 
                target='test_cnpy',
                includes=".",
                lib = ["boost_thread", "boost_system", "pthread","z"],
                use='EIGEN BOOST',
    )    
    bld.program(source='maparray.cpp', 
                target='maparray',
                includes=".",
                lib = ["boost_thread", "boost_system", "pthread","rt"],
                use=['EIGEN','BOOST']
    )    

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
