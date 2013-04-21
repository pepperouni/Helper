#!/usr/bin/python
import HTMLParser
import urllib2, commands, os,sys

#Download
def download(URL,Target):
	try:
		tmphandler=urllib2.urlopen(URL)
		out = tmphandler.info().headers
		found = 0
		Type=""
		for i in range(0,len(tmphandler.info().headers)):
			if 'Content-Type:' in tmphandler.info().headers[i]:
				s = tmphandler.info().headers[i].index('Content-Type:')+len('Content-Type:')
				try:
					e = (tmphandler.info().headers[i]).index(";")
				except ValueError, ex:
					e = len(tmphandler.info().headers[i])
				Type = (tmphandler.info().headers[i])[s+1:e]
				break
		if Type.lower().strip()!="text/html":
			return 0
	except ValueError, ex:
		return 0;
	except urllib2.URLError,TypeError:
		return 0;
	filetmphandler=open(Target,"wb")
	filetmphandler.write(tmphandler.read())
	filetmphandler.close()
	return 1
	

def everything_between(text,begin,end):
    idx1=text.find(begin)
    idx2=text.find(end,idx1)
    return text[idx1+len(begin):idx2].strip()
    
def Title(url):
	if (download(url,"Handler.html")==0):
		return "#ERROR"
	
	title="#ERROR"	
	content=open('Handler.html').read()
	if '<title' in content and '</title>' in content:
		title=everything_between(content,'<title>','</title>')
	
	os.system("rm Handler.html")
	return title

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

def do():
	if (len(sys.argv)<2):
		print "#ERROR"
		return
	
	Parameter=sys.argv[1]
	
	html_parser = HTMLParser.HTMLParser()
	
	if 'http' not in Parameter:
		Parameter="http://"+Parameter
	
	if (is_url(Parameter)==1):
		string = html_parser.unescape(Title(Parameter))
		print string.encode('utf-8')
	else:
		print "#ERROR"
		return

do()
