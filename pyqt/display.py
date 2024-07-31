import sys
from PyQt5.QtWidgets import QApplication, QLabel, QMainWindow
from PyQt5.QtGui import QPixmap
import os

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Image Display")
        self.setGeometry(20, 20, 20, 20)  

        # Set the image file path
        image_path = os.path.join(r"C:\anconda_python\ROS", "panel_design.jpg") #paCopy and paste the path of the image file
        
        label = QLabel(self)
        pixmap = QPixmap(image_path)
        label.setPixmap(pixmap)
        label.resize(pixmap.width(), pixmap.height())
        
        scaled_pixmap = pixmap.scaled(1000, 600)
        label.setPixmap(scaled_pixmap)
        label.resize(2000, 2000)
        
        self.setCentralWidget(label)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())

