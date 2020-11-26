import os
import hashlib
from PIL import Image, ImageDraw

region =  {
    "top": {
        "x": 0,
        "y": 0,
        "w": 64,
        "h": 31
    },
    "mid": {
        "x": 0,
        "y": 31,
        "w": 64,
        "h": 29    
    },
    "bot": {
        "x": 0,
        "y": 60,
        "w": 64,
        "h": 32        
    }
}
frame_size = {
	"w": 64,
	"h": 92
}
output_size = {
    "w": frame_size["w"] * 10,
    "h": frame_size["w"] * 10,
}
output_scale = {
    "x": 2,
    "y": 2
}
hands_size = {
    "w": 24,
    "h": 24
}
standing = [ "standing_1.png" ]
walking = [ "walking_1.png", "walking_2.png" ]
hands = [ "hand_closed.png", "hand_open.png" ]
output = "composed.png"

img = Image.new('RGB', (output_size["w"], output_size["h"]), color = 'red')

xcursor = 0
ycursor = 0

### bot
for file in standing:
    reg = region["bot"]
    c_dims = (reg["x"], reg["y"], reg["x"] + reg["w"], reg["y"] + reg["h"])
    frame = Image.open(file).convert("RGB")
    ImageDraw.floodfill(frame, xy=(0,0), value=(255, 0, 0), thresh=50)
    img.paste(frame.crop(c_dims), (xcursor, ycursor + reg["y"]))
    ycursor += frame_size["h"]

for file in walking:
    reg = region["bot"]
    c_dims = (reg["x"], reg["y"], reg["x"] + reg["w"], reg["y"] + reg["h"])
    frame = Image.open(file).convert("RGB")
    ImageDraw.floodfill(frame, xy=(0,0), value=(255, 0, 0), thresh=50)
    ImageDraw.floodfill(frame, xy=(30,66), value=(255, 0, 0), thresh=50) # in between legs
    img.paste(frame.crop(c_dims), (xcursor, ycursor + reg["y"]))
    xcursor += frame_size["w"]    
    
xcursor = 0
ycursor += frame_size["h"]

### mid
for file in standing:
    reg = region["mid"]
    c_dims = (reg["x"], reg["y"], reg["x"] + reg["w"], reg["y"] + reg["h"])
    frame = Image.open(file).convert("RGB")
    ImageDraw.floodfill(frame, xy=(0,0), value=(255, 0, 0), thresh=50)
    img.paste(frame.crop(c_dims), (xcursor, ycursor + reg["y"]))
    ycursor += frame_size["h"]

### top
for file in standing:
    reg = region["top"]
    c_dims = (reg["x"], reg["y"], reg["x"] + reg["w"], reg["y"] + reg["h"])
    frame = Image.open(file).convert("RGB")
    ImageDraw.floodfill(frame, xy=(0,0), value=(255, 0, 0), thresh=50)
    img.paste(frame.crop(c_dims), (xcursor, ycursor + reg["y"]))
    
xcursor = frame_size["w"]
ycursor = 0

### hands
for file in hands:
    frame = Image.open(file).convert("RGB")
    ImageDraw.floodfill(frame, xy=(0,0), value=(255, 0, 0), thresh=50)
    img.paste(frame, (xcursor, ycursor))
    xcursor += hands_size["w"]

#img.resize((output_size["w"] * 2, output_size["h"] * 2), Image.NEAREST).save(output)
img.save(output)
#print("saved. md5: %s" % (hashlib.md5(open(output,'rb').read()).hexdigest()))
