#! /usr/bin/env python
# encoding: utf-8
# WARNING! Do not edit! https://waf.io/book/index.html#_obtaining_the_waf_file

import os,re
from waflib import Task,TaskGen,Logs
def configure(conf):
	conf.find_program('python',var='PY2CMD',mandatory=False)
	conf.find_program('python3',var='PY3CMD',mandatory=False)
	if not conf.env.PY2CMD and not conf.env.PY3CMD:
		conf.fatal("No Python interpreter found!")
class run_py_2_script(Task.Task):
	run_str='${PY2CMD} ${SRC[0].abspath()}'
	shell=True
class run_py_3_script(Task.Task):
	run_str='${PY3CMD} ${SRC[0].abspath()}'
	shell=True
@TaskGen.feature('run_py_script')
@TaskGen.before_method('process_source')
def apply_run_py_script(tg):
	v=getattr(tg,'version',3)
	if v not in(2,3):
		raise ValueError("Specify the 'version' attribute for run_py_script task generator as integer 2 or 3.\n Got: %s"%v)
	src_node=tg.path.find_resource(tg.source)
	tgt_nodes=[tg.path.find_or_declare(t)for t in tg.to_list(tg.target)]
	tsk=tg.create_task('run_py_%d_script'%v,src=src_node,tgt=tgt_nodes)
	tsk.env.env=dict(os.environ)
	tsk.env.env['PYTHONPATH']=tsk.env.env.get('PYTHONPATH','')
	project_paths=getattr(tsk.env,'PROJECT_PATHS',None)
	if project_paths and'PROJECT_ROOT'in project_paths:
		tsk.env.env['PYTHONPATH']+=os.pathsep+project_paths['PROJECT_ROOT'].abspath()
	if getattr(tg,'add_to_pythonpath',None):
		tsk.env.env['PYTHONPATH']+=os.pathsep+tg.add_to_pythonpath
	tsk.env.env['PYTHONPATH']=re.sub(os.pathsep+'+',os.pathsep,tsk.env.env['PYTHONPATH'])
	if tsk.env.env['PYTHONPATH'].startswith(os.pathsep):
		tsk.env.env['PYTHONPATH']=tsk.env.env['PYTHONPATH'][1:]
	for x in tg.to_list(getattr(tg,'deps',[])):
		node=tg.path.find_resource(x)
		if not node:
			tg.bld.fatal('Could not find dependency %r for running %r'%(x,src_node.abspath()))
		tsk.dep_nodes.append(node)
	Logs.debug('deps: found dependencies %r for running %r',tsk.dep_nodes,src_node.abspath())
	tg.source=[]
