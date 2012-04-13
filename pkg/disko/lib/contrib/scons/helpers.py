#!/usr/bin/env python

import os, fnmatch, string

def findFiles(searchDir, pattern):
	files = []
	for entry in os.listdir(searchDir):
		entrypath = os.path.join(searchDir, entry)
		if os.path.isfile(entrypath) and fnmatch.fnmatch(entry, pattern):
			files.append(entrypath)
		elif os.path.isdir(entrypath):
			files.extend(findFiles(entrypath, pattern))

	return files