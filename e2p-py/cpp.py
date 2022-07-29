import time
import math

import numpy as np
import matplotlib.pyplot as plt
import cv2

THETA = 120
PHI = 0
FOV = 90
Hd = 320
Wd = 320

frame = cv2.imread("P_1024.jpg")

equ_h, equ_w = frame.shape[:2]
equ_cx, equ_cy = equ_w / 2.0, equ_h / 2.0

wFOV = FOV
hFOV = Hd * 1.0 / Wd * wFOV

w_len = 2 * math.tan(wFOV * math.pi / 360.0)
h_len = 2 * math.tan(hFOV * math.pi / 360.0)

map_x = np.zeros((Hd, Wd), dtype="float32")
map_y = np.zeros((Hd, Wd), dtype="float32")

x_, y_, z_ = 0, 0, 0
XYZ = np.zeros((3,1), dtype="float32")

t1 = time.time()

beta = -PHI * math.pi / 180.0
gamma = -THETA * math.pi / 180.0

Rx = np.array([1,0,0,0,math.cos(0),-1*math.sin(0),0,math.sin(0),math.cos(0)], dtype="float32").reshape(3,3)
Ry = np.array([math.cos(beta), 0, -1 * math.sin(beta), 0, 1, 0, math.sin(beta), 0, math.cos(beta)], dtype="float32").reshape(3,3)
Rz = np.array([math.cos(gamma), -1 * math.sin(gamma), 0, math.sin(gamma), math.cos(gamma), 0, 0, 0, 1], dtype="float32").reshape(3,3)
R = np.matmul(np.matmul(Rx, Ry), Rz)

for x in range(Wd):
    for y in range(Hd):
        x_ = 1.0
        y_ = (x * 1.0 / Wd - 0.5) * w_len
        z_ = (y * 1.0 / Hd - 0.5) * h_len

        r = math.sqrt(x_ * x_ + y_ * y_ + z_ * z_)

        XYZ[0, 0] = x_ / r
        XYZ[1, 0] = y_ / r
        XYZ[2, 0] = z_ / r

        XYZ = np.matmul(R, XYZ)
        x_ = XYZ[0, 0]
        y_ = XYZ[1, 0]
        z_ = XYZ[2, 0]

        lon = (math.asin(z_) / math.pi + 0.5) * equ_h
        lat = (math.atan2(y_, x_ + 0.01) / (2 * math.pi) + 0.5) * equ_w

        map_x[y, x] = lat
        map_y[y, x] = lon

outFrame = np.zeros((Hd, Wd))

outFrame = cv2.remap(frame, map_x, map_y, interpolation=0, borderMode=2)

t2 = time.time()
print(t2-t1)

plt.imshow(outFrame[...,::-1])
plt.show()

