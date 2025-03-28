#! /usr/bin/env python
# encoding: utf-8
# WARNING! Do not edit! https://waf.io/book/index.html#_obtaining_the_waf_file

from waflib.Utils import def_attrs
from waflib import Task,Options
from waflib.TaskGen import feature,after_method
class objcopy(Task.Task):
	run_str='${OBJCOPY} -O ${TARGET_BFDNAME} ${OBJCOPYFLAGS} ${SRC} ${TGT}'
	color='CYAN'
@feature('objcopy')
@after_method('apply_link')
def map_objcopy(self):
	def_attrs(self,objcopy_bfdname='ihex',objcopy_target=None,objcopy_install_path="${PREFIX}/firmware",objcopy_flags='')
	link_output=self.link_task.outputs[0]
	if not self.objcopy_target:
		self.objcopy_target=link_output.change_ext('.'+self.objcopy_bfdname).name
	task=self.create_task('objcopy',src=link_output,tgt=self.path.find_or_declare(self.objcopy_target))
	task.env.append_unique('TARGET_BFDNAME',self.objcopy_bfdname)
	try:
		task.env.append_unique('OBJCOPYFLAGS',getattr(self,'objcopy_flags'))
	except AttributeError:
		pass
	if self.objcopy_install_path:
		self.add_install_files(install_to=self.objcopy_install_path,install_from=task.outputs[0])
def configure(ctx):
	program_name='objcopy'
	prefix=getattr(Options.options,'cross_prefix',None)
	if prefix:
		program_name='{}-{}'.format(prefix,program_name)
	ctx.find_program(program_name,var='OBJCOPY',mandatory=True)
