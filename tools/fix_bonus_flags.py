from PIL import Image
import os
from collections import deque

def key_with_floodfill(src_img):
    rgb = src_img.convert('RGB')
    rgba = src_img.convert('RGBA')
    px = rgba.load()
    rgbpx = rgb.load()
    w, h = rgb.size

    visited = [[False]*h for _ in range(w)]
    q = deque()
    for x in range(w):
        for y in (0, h-1):
            r,g,b = rgbpx[x,y]
            if r >= 250 and g >= 250 and b >= 250 and not visited[x][y]:
                visited[x][y] = True
                q.append((x, y))
    for y in range(h):
        for x in (0, w-1):
            r,g,b = rgbpx[x,y]
            if r >= 250 and g >= 250 and b >= 250 and not visited[x][y]:
                visited[x][y] = True
                q.append((x, y))
    while q:
        x, y = q.popleft()
        px[x,y] = (0,0,0,0)
        for nx, ny in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
            if 0 <= nx < w and 0 <= ny < h and not visited[nx][ny]:
                r,g,b = rgbpx[nx,ny]
                if r >= 250 and g >= 250 and b >= 250:
                    visited[nx][ny] = True
                    q.append((nx, ny))
    return rgba

src_dir = r'd:\LEGOWB-Project\00WBWORKFOLDER\assets\cast_ripper_d12\worldbuilder'
dst_dir = r'd:\PsyDoom-Project\LEGOWB3DS\gfx'

flag_map = [
    ('title and levels_66_bonus_flag1.bmp', 'bonus_flag1.png'),
    ('title and levels_44_bonus_flag2.bmp', 'bonus_flag2.png'),
    ('title and levels_46_bonus_flag3.bmp', 'bonus_flag3.png'),
    ('title and levels_48_bonus_flag4.bmp', 'bonus_flag4.png'),
    ('title and levels_50_bonus_flag5.bmp', 'bonus_flag5.png'),
    ('title and levels_52_bonus_flag6.bmp', 'bonus_flag6.png'),
    ('title and levels_145_ocean_bonus_flag1.bmp', 'ocean_bonus_flag1.png'),
    ('title and levels_146_ocean_bonus_flag2.bmp', 'ocean_bonus_flag2.png'),
    ('title and levels_147_ocean_bonus_flag3.bmp', 'ocean_bonus_flag3.png'),
    ('title and levels_148_ocean_bonus_flag4.bmp', 'ocean_bonus_flag4.png'),
    ('title and levels_149_ocean_bonus_flag5.bmp', 'ocean_bonus_flag5.png'),
    ('title and levels_150_ocean_bonus_flag6.bmp', 'ocean_bonus_flag6.png'),
]

for src_name, dst_name in flag_map:
    src_path = os.path.join(src_dir, src_name)
    dst_path = os.path.join(dst_dir, dst_name)
    src_img = Image.open(src_path)
    result = key_with_floodfill(src_img)
    result.save(dst_path)
    print('Fixed:', dst_name)
