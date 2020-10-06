import sys
import imageio
import numpy as np
import xml.etree.cElementTree as ET
tree = ET.parse('src/roboto_condensed_bold_48.xml')
im = imageio.imread('src/roboto_condensed_bold_48.png')
im = np.asarray(im)
n = 0
widths = []
symbols = ""
for c in tree.findall("Char"):
    name = c.get('code')
    if name == "&quot;":
        name = "\""
    if name == "&amp;":
        name = "&"
    if name == "&lt;":
        name = "<"
    x, y, w, h = map(int, c.get('rect').split(' '))
    widths.append(w)
    symbols += name
    o = int(c.get('offset').split(' ')[1])
    char_img = (im[y:y+h, x:x+w, 3] > 0).astype(int)
    char_img = np.flip(np.pad(
        char_img.T, ((0, 0), (5+o, 128-h-5-o)), mode='constant'), 0)
    np.packbits(char_img).tofile('data/font/%d.bin' % n)
    n += 1
print(widths)
print(symbols)
