import sys
import imageio
import numpy as np
import xml.etree.cElementTree as ET
tree = ET.parse('src/roboto_condensed_bold_48.xml')
char_options = tree.find("Char[@code='A']")
x, y, w, h = map(int, char_options.get('rect').split(' '))
im = imageio.imread('src/roboto_condensed_bold_48.png')
im = np.asarray(im)
print(x, y, w, h)
np.set_printoptions(threshold=sys.maxsize)
char_img = (im[y:y+h, x:x+w, 3] > 0).astype(int)
transposed_padded = np.pad(char_img.T, ((0, 0), (8-h % 8, 0)), mode='constant')
np.packbits(transposed_padded).tofile('A.bin')
# print(np.packbits(char_img.T))
