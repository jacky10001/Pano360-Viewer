
from PyQt5 import QtCore, QtGui, QtWidgets
from Ui_main import Ui_MainWindow

import numpy as np
from PIL import Image

import py360convert

import time


class args:
    i = "P_1024.jpg"
    h_fov = 60
    v_fov = 60
    u_deg = 50
    v_deg = 0
    in_rot_deg = 0
    mode = 'bilinear'
    h = 300
    w = 300
    o = "out.jpg"


MODE_IMAGE = 0
MODE_PLANE = 1

MOUSE_NULL = 0
MOUSE_LEFT = 1
MOUSE_RIGHT = 2


class MainWindow(QtWidgets.QMainWindow, Ui_MainWindow):
    already = False
    mode = 0  # 0: image , 1: plane
    def setModeToImage(self): self.mode = MODE_IMAGE
    def setModeToPlane(self): self.mode = MODE_PLANE
    def getMode(self): return self.mode

    u_x1 = 0
    u_x2 = 0
    v_y1 = 0
    v_y2 = 0

    __mouse_btn = 0  # 0: null , 1: left , 2: right
    def setPressMouseNull(self): self.__mouse_btn = MOUSE_NULL
    def setPressMouseLefttButton(self): self.__mouse_btn = MOUSE_LEFT
    def setPressMouseRightButton(self): self.__mouse_btn = MOUSE_RIGHT
    def getPressMouseButtonStatus(self): return self.__mouse_btn

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupUi(self)
        self.setupToolBar()
        self.toolBar.actionTriggered[QtWidgets.QAction].connect(
            self.triggerReceiver)
        self.viewData.installEventFilter(self)
        self.viewData.setMouseTracking(True)

    def setupToolBar(self):
        self.toolBarActAdd = self.toolBar.addAction("Load")
        self.toolBarActAdd = self.toolBar.addAction("Show e")
        self.toolBarActAdd = self.toolBar.addAction("Show p")

    def triggerReceiver(self, act):
        act_type = act.text()
        if act_type == "Load":
            self.readImage()
        if act_type == "Show e":
            self.showImage()
        if act_type == "Show p":
            self.showPlane()

    def readImage(self):
        self.already = True
        self.image = np.array(Image.open(args.i))
        self.showMsg("File path:", args.i)

    def showImage(self):
        if not self.already:
            return
        self.setModeToImage()
        Ny, Nx, Nc = self.image.shape
        bytes_per_line = Nx * Nc
        qimg = QtGui.QImage(self.image.data, Nx, Ny,
                            bytes_per_line, QtGui.QImage.Format_RGB888)
        self.viewData.setPixmap(QtGui.QPixmap.fromImage(qimg))
        self.showMsg("Width: {} Height: {} Channel: {}".format(Nx, Ny, Nc))

    def showPlane(self):
        if not self.already:
            return
        self.setModeToPlane()
        img = self.image.copy()
        t1 = time.time()
        out = py360convert.e2p(
            img, fov_deg=(
                args.h_fov, args.v_fov), u_deg=args.u_deg, v_deg=args.v_deg,
            out_hw=(args.h, args.w), in_rot_deg=args.in_rot_deg, mode=args.mode)
        t2 = time.time()
        # print(t2-t1)
        out = out.astype(np.uint8)
        Ny, Nx, Nc = out.shape
        bytes_per_line = Nx * Nc
        qimg = QtGui.QImage(out.data, Nx, Ny, bytes_per_line,
                            QtGui.QImage.Format_RGB888)
        self.viewData.setPixmap(QtGui.QPixmap.fromImage(qimg))
        self.showMsg("u_deg: {} v_deg: {} h_fov: {} v_fov: {}".format(
            args.u_deg, args.v_deg, args.h_fov, args.v_fov))

    def eventFilter(self, source, event):
        if self.getMode() == MODE_PLANE:
            ##
            # Detect mouse button
            ##
            if event.type() == QtCore.QEvent.MouseButtonPress:
                if event.button() == QtCore.Qt.LeftButton:
                    self.setPressMouseLefttButton()
                elif event.button() == QtCore.Qt.RightButton:
                    self.setPressMouseRightButton()
            ##
            # Tracking mouse position
            ##
            if source == self.viewData:
                # angle view
                if self.getPressMouseButtonStatus() == MOUSE_LEFT:
                    if event.type() == QtCore.QEvent.MouseButtonPress:
                        self.u_x1 = event.pos().x()
                        self.v_y1 = event.pos().y()
                    elif event.type() == QtCore.QEvent.MouseMove:
                        self.u_x2 = event.pos().x()
                        self.v_y2 = event.pos().y()
                        args.u_deg -= (self.u_x2 - self.u_x1)//40
                        args.v_deg += (self.v_y2 - self.v_y1)//40
                        if args.v_deg > 90:
                            args.v_deg = 90
                        if args.v_deg < -90:
                            args.v_deg = -90
                        self.showPlane()
                    elif event.type() == QtCore.QEvent.MouseButtonRelease:
                        self.setPressMouseNull()
                # field view
                if event.type() == QtCore.QEvent.Wheel:
                    numDegrees = event.angleDelta().y() / 8
                    numSteps = numDegrees // 15
                    args.h_fov -= numSteps * 2
                    args.v_fov -= numSteps * 2
                    if args.h_fov > 100:
                        args.h_fov = 100
                    if args.v_fov > 100:
                        args.v_fov = 100
                    if args.h_fov < 20:
                        args.h_fov = 20
                    if args.v_fov < 20:
                        args.v_fov = 20
                    self.showPlane()
        return QtWidgets.QWidget.eventFilter(self, source, event)

    def showMsg(self, *msg, time=3000):
        msg_str = " ".join(map(lambda s: str(s), msg))
        self.statusBar.showMessage(msg_str, time)


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
