#! /usr/bin/env python
# encoding: utf-8
# WARNING! Do not edit! https://waf.io/book/index.html#_obtaining_the_waf_file

__author__=__maintainer__="Alibek Omarov <a1ba.omarov@gmail.com>"
__copyright__="Alibek Omarov, 2019"
import sys
from waflib import Logs
class ColorMSVCFormatter(Logs.formatter):
	def __init__(self,colors):
		self.colors=colors
		Logs.formatter.__init__(self)
	def parseMessage(self,line,color):
		arr=line.split(':',3)
		if len(arr)<4:
			return line
		colored=self.colors.BOLD+arr[0]+':'+arr[1]+':'+self.colors.NORMAL
		colored+=color+arr[2]+':'+self.colors.NORMAL
		colored+=arr[3]
		return colored
	def format(self,rec):
		frame=sys._getframe()
		while frame:
			func=frame.f_code.co_name
			if func=='exec_command':
				cmd=frame.f_locals.get('cmd')
				if isinstance(cmd,list):
					argv0=cmd[0].lower()
					if'cl.exe'in argv0:
						lines=[]
						for line in rec.msg.splitlines():
							if': warning 'in line:
								lines.append(self.parseMessage(line,self.colors.YELLOW))
							elif': error 'in line:
								lines.append(self.parseMessage(line,self.colors.RED))
							elif': fatal error 'in line:
								lines.append(self.parseMessage(line,self.colors.RED+self.colors.BOLD))
							elif': note: 'in line:
								lines.append(self.parseMessage(line,self.colors.CYAN))
							else:
								lines.append(line)
						rec.msg="\n".join(lines)
			frame=frame.f_back
		return Logs.formatter.format(self,rec)
def options(opt):
	Logs.log.handlers[0].setFormatter(ColorMSVCFormatter(Logs.colors))
