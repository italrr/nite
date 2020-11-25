import os
from PIL import Image, ImageDraw

files = []
size = {"w": 64, "h": 64}
output = "../general_icons.png"
target_size = {"w": size["w"] * 8, "h": size["h"] * 8}
ignores = ["./", "_frame.png", "build.py"]

for file in os.listdir("./"):
    if file in ignores:
        continue
    files.append(file)


img = Image.new('RGBA', (target_size["w"], target_size["h"]), color = 'white')

xcursor = 0
ycursor = 0

for file in files:
    
    current = Image.open(file)
    img.paste(current, (xcursor, ycursor))
    xcursor += size["w"]
    if xcursor > target_size["w"]:
        xcursor = 0;
        ycursor += size["h"]
    
img.save(output)

print("wrote %s" % (output))