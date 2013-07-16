#!/usr/bin/python
import urllib2, commands, os,sys

#Download
def download(URL,Target):
	try:
		tmphandler=urllib2.urlopen(URL)		
	except ValueError, ex:
		return 0;
	except urllib2.URLError, exceptions.TypeError:
		return 0;
	filetmphandler=open(Target,"wb")
	filetmphandler.write(tmphandler.read())
	filetmphandler.close()
	return 1
#Shrink function
def Shrink(URL):
	Link=str("http://tinyurl.com/create.php?source=indexpage&url=")+str(URL)+str("&submit=Make+TinyURL!&alias=")
	if (download(Link,"Handler.html")==0):
		return "#ERROR"
	GotLink=""
	file = open("Handler.html", 'r+')
	while 1:
		line = file.readline()
		if '</b></blockquote>' in line:
			line = file.readline()
			line = file.readline()
			Start=line.index('<blockquote><b>')+len('<blockquote><b>');
			End=line.index('</b><div id="')
			GotLink=line[Start:End]
			break
		if line=="":
			break;
	os.system("rm Handler.html")
	return GotLink
def is_url(url):
	true = 0
	try:
		if (url.index("www")==0):
			true = 1
	except ValueError, ex:
		true = 0
	try:	
		if (url.index("http")==0):
			true = 1		
	except ValueError, ex:
		true = 1	
	return true	
		
		
	return true
def do():
	if (len(sys.argv)<2):
		print "#ERROR"
		return
	
	Parameter=sys.argv[1]
	
	if (is_url(Parameter)==1):
		print Shrink(Parameter)
	else:
		print "#ERROR"
		return

do()
