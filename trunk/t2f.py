# Super-flaky script using ImageMagick's convert to, well, convert
# truetype fonts to Lecturer's preferred format.
# Creates 8859-1 codepoints on an UTF-8 system, for some reason.

import os
import struct
import sys
import time 
minchar = 32
maxchar = 256

for k in [8, 10, 12, 14, 16]:
  pos = 0
  char = []
  for j in range(minchar,maxchar):
    i = chr(j)
    if i == '\\': i = '\\\\'
    if i != "'":
      a = "convert -font CharterBT-Roman.ttf -pointsize " + str(k) + " 'label:" + i + "' -identify -format gray -depth 8 x.gray"
    else:
      a = 'convert -font CharterBT-Roman.ttf -pointsize ' + str(k) + ' "label:' + i + '" -identify -format gray -depth 8 x.gray'
    
    #p = subprocess.Popen(a,shell=True,stdout=subprocess.PIPE).stdout
    sin,sout = os.popen2(a, 'r')
    l = sout.readline().split(' ')
    print l
    if i == ' ':
      x,y = l[4].split('x')
    else:
      x,y = l[2].split('x')
    print i,'x',x,'y',y
    sin.close()
    sout.close()
    time.sleep(.1)
    data = open('x.gray').read()
    os.unlink('x.gray')
    if len(data) != int(x)*int(y):
      print 'expected',int(x)*int(y),'bytes, got',len(data)
      sys.exit(1)
    char += [[int(x),int(y),pos,data]]
    pos += len(data)

  pos = len(char) * 8 + 4 + 4

  out = open('fonts/font'+str(k),'w')
  out.write(struct.pack('<ii', minchar, maxchar))
  for j in char:
    out.write(struct.pack('<HHi', j[0],j[1],pos+j[2]))
  for j in char:
    out.write(j[3])
  out.close()
