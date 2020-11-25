import os
from PIL import Image, ImageDraw

files = []
target = "data/texture/overworld/_humanoid"
output = "composed.png"
use_loose_limbs = True

hands = [
    
]

top = [

]

mid = [

]

bottom = [

]
   
for file in os.listdir(target):
    if file in ignores:
        continue
    files.append(file)

img = Image.new('RGBA', (imgsize[0], imgsize[1]), color = 'white')
    
for file



