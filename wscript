
import sys

def options(opt):
        opt.load('compiler_c')


contribs = [
('clinkedlistqueue', 'git@github.com:willemt/CLinkedListQueue.git'),
('ccircularbuffer', 'git@github.com:willemt/CCircularBuffer.git'),
('chashmap_via_linked_list','git@github.com:willemt/CHashMapViaLinkedList.git'),
('cmeanqueue','git@github.com:willemt/CMeanQueue.git'),
('cheap','git@github.com:willemt/CHeap.git'),
('carraylistf','git@github.com:willemt/CFixedArraylist.git'),
('pseudolru','git@github.com:willemt/CPseudoLRU.git')]

def configure(conf):
    conf.load('compiler_c')

    if sys.platform == 'darwin':
        conf.env.LIBPATH = [
		'/System/Library/Frameworks/OpenGL.framework/Libraries',
		'/opt/local/lib/'
	]
        conf.env.INCLUDES = [
		'/System/Library/Frameworks/OpenGL.framework/Libraries',
		 '/opt/local/include'
	]

    conf.check_cfg(path='sdl-config', args='--cflags --libs', package='', uselib_store='SDL')
    conf.check_cc(lib='GL')
#    conf.check_cc(lib='glut')
#    conf.check_cc(lib='GLEW')
#    conf.check_cc(lib='SDL')
    conf.check_cc(lib='SDL_image')

    if sys.platform != 'win32':
            conf.env.LIB       = ['m','SDL_image']#,']#glut']
#            conf.env.CFLAGS   = ['-O0'] 
#            conf.env.LIBPATH = ['/usr/lib']
#            conf.env.LINKFLAGS_TEST = ['-g']
#            conf.env.INCLUDES_TEST  = ['/opt/gnome/include']

    return

    # Get the required contributions via GIT
    for c in contribs:
        print "Git pulling %s..." % c[1]
        conf.exec_command("mkdir %s" % c[0])
        conf.exec_command("git init", cwd=c[0])
        conf.exec_command("git pull %s" % c[1], cwd=c[0])


from waflib.Task import Task
class compiletest(Task):
        def run(self):
                return self.exec_command('sh ../make-tests.sh %s > %s' % (
                                self.inputs[0].abspath(),
                                self.outputs[0].abspath()
                        )
                )

def unittest(bld, src):
        bld(rule='sh ../make-tests.sh ${SRC} > ${TGT}', source=src, target="t"+src)
        bld.program(
                source='%s %s CuTest.c' % (src,"t"+src),
                target=src[:-2],
                cflags=[
                    '-g',
                    '-Werror'
                ],
                use='tearen',
                unit_test='yes',
                includes='.')

        bld(rule='pwd && export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. && ./${SRC}', source=src[:-2])


#*----------------------------------------------------------------------------*/

def build(bld):
        bld.shlib(
            source="""
            r_draw.c
            r_media.c
            r_texture_atlas.c
            r_object.c
            r_vbo.c
            r_vbo_managed.c
            r_util.c
            test_tearen.c
            CuTest.c
            chashmap_via_linked_list/linked_list_hashmap.c
            clinkedlistqueue/linked_list_queue.c
            cheap/heap.c
            pseudolru/pseudolru.c
            carraylistf/fixed_arraylist.c
            """,
                target='tearen',
                use=['GL', 'GLEW'],#, 'SDL', 'SDL_image'],
                includes='clinkedlistqueue chashmap_via_linked_list cheap pseudolru carraylistf',
                uselib = 'SDL',
                cflags=[
                    '-Werror',
                    '-g',
                    '-O',
                    '-Werror=uninitialized',
                    '-Werror=return-type',
                    '-Wcast-align']
                )


        unittest(bld,'test_texture_atlas.c')
        bld.program(

                source='demo.c',
                target='demo',
                cflags=[
                    '-g',
                    '-O',
                    '-Werror',
                    '-Werror=uninitialized',
                    '-Werror=return-type'
                    ],
                use=['tearen'])

