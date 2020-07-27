from PIL import Image, ImageDraw

rel_path = "data/texture/overworld/humanoid_sliced"
framesize = [128, 192]
cells = [5, 7]
filename = "player_sheet.png"
imgsize = framesize * cells

bot_walk = [
	"bot_walking_1.png",
	"bot_walking_2.png"
]

bot_standing = [
	"bot_standing.png"
]

mid_punching = [
	"mid_punching_1.png",
	"mid_punching_2.png"
]

mid_standing = [
	"mid_standing.png"
]

mid_parry = [
	"mid_hand2hand_parry.png"
]

mid_casting = [
	"mid_cast.png"
]

top_neutral = [
	"top_neutral.png"
]


list = [
	bot_walk,
	bot_standing,
	mid_punching,
	mid_standing,
	mid_parry,
	mid_casting,
	top_neutral,
]

def paste(list, path, to, x, y, hor):
	global framesize
	for idx,item in enumerate(list):
		p = (x + (framesize[0] * idx * 1 if not hor else 0), y + (framesize[1] * idx * 1 if hor else 0))
		img = Image.open(path + "/" + item).resize((framesize[0],framesize[1]), Image.NEAREST)
		to.paste(img, p)

# build
img = Image.new('RGBA', (imgsize[0], imgsize[1]), color = 'white')

for i,item in enumerate(list):
	paste(list[i], rel_path, img, 0, framesize[1] * i, False)


#save it
img.save(rel_path + "/../" + filename)


	
	
	
	
