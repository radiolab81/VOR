#!/usr/bin/env python

import time
import sys
import xmlrpclib

#create server object
s = xmlrpclib.Server("http://127.0.0.1:7777")

print sys.argv[1]
s.set_freq(float(sys.argv[1]))

