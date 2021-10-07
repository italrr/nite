import os
from PIL import Image, ImageDraw

dirs = [
    "STANDING/RIGHT",
    "STANDING/LEFT",
    "STANDING/DOWN",
    "STANDING/UP",
    
    "STANDING/UP_RIGHT",
    "STANDING/DOWN_RIGHT",
    
    "STANDING/UP_LEFT",
    "STANDING/DOWN_LEFT",
    
    "WALKING/RIGHT",
    "WALKING/LEFT",
    "WALKING/DOWN",
    "WALKING/UP",
    
    "WALKING/UP_RIGHT",
    "WALKING/DOWN_RIGHT",
    
    "WALKING/UP_LEFT",
    "WALKING/DOWN_LEFT"    
]

frame_number = 10
anim_angles = 8
anim_states = 2
front_face = [100, 100]
front_stand = [600, 1100]
input_size = [600, 900]
output_size = [input_size[0] * frame_number + front_face[0] + front_stand[0], input_size[1] * anim_angles * anim_states]
output_filename = "final.png"
output_scale = 0.25

def process_dir(dir, target, y):
    cursor_x = 0
    files = os.listdir(f'{dir}/')
    for img in files:
        path = f'{dir}/{img}'
        current = Image.open(path)
        target.paste(current, (cursor_x, y))
        cursor_x += input_size[0]

def paste_at(dir, target, x, y):
    current = Image.open(dir)
    target.paste(current, (x, y))
    
target = Image.new('RGBA', (output_size[0], output_size[1]), (255, 255, 255, 0))
cursor_y = 0
for dir in dirs:
    process_dir(dir, target, cursor_y)
    cursor_y += input_size[1]

paste_at("FRONTS/FACE/0001.png", target, input_size[0] * frame_number, 0)
fx = input_size[0] * frame_number * output_scale
fy = 0
fw = front_face[0] * output_scale
fh = front_face[1] * output_scale
print(f"pasted face x:{fx} y:{fy} w:{fw} h:{fh}")
paste_at("FRONTS/STAND/0001.png", target, input_size[0] * frame_number + front_face[0], 0)
fx = (input_size[0] * frame_number + front_face[0]) * output_scale
fy = 0
fw = front_stand[0] * output_scale
fh = front_stand[1] * output_scale
print(f"pasted face x:{fx} y:{fy} w:{fw} h:{fh}")



final = target.resize((int(output_size[0] * output_scale), int(output_size[1] * output_scale)), Image.ANTIALIAS)
final.save(output_filename, 'PNG')

#w 150
#h 225