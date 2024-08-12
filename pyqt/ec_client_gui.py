import sys
import os
import socket
import threading
from PyQt5.QtWidgets import QApplication, QWidget, QGridLayout, QVBoxLayout, QLabel, QPushButton, QDial, QLineEdit, QGraphicsView, QGraphicsScene, QFrame, QDialog, QTextEdit
from PyQt5.QtCore import Qt, QSize, pyqtSignal, QObject
from PyQt5.QtGui import QIcon, QPixmap

class ConnectionDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Connection")
        layout = QVBoxLayout()

        self.ip_input = QLineEdit(self)
        self.ip_input.setPlaceholderText("IP Address")
        layout.addWidget(self.ip_input)

        self.port_input = QLineEdit(self)
        self.port_input.setPlaceholderText("Port")
        layout.addWidget(self.port_input)

        connect_button = QPushButton("Connect", self)
        connect_button.clicked.connect(self.accept)
        layout.addWidget(connect_button)

        self.setLayout(layout)

    def get_connection_info(self):
        return self.ip_input.text(), self.port_input.text()

class SocketSignals(QObject):
    receivedData = pyqtSignal(str)

class SkeletonPanel(QWidget):
    def __init__(self):
        super().__init__()
        self.is_auto = False
        self.initUI()
        self.socket = None
        self.signals = SocketSignals()
        

    def initUI(self):
        self.setWindowTitle('Skeleton Panel')
        self.setGeometry(100, 100, 1200, 800)

        # Set background image
        self.image_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'doc/background.jpg'))
        self.background = QGraphicsView(self)
        self.scene = QGraphicsScene()
        self.background.setScene(self.scene)
        self.background.setStyleSheet("background: transparent; border: none;")
        self.background.setGeometry(0, 0, 1200, 800)
        self.bg_image = QPixmap(self.image_path).scaled(1200, 800, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
        self.scene.addPixmap(self.bg_image)

        # Main layout
        layout = QGridLayout()
        self.setLayout(layout)

        # Frame for Robot Camera View
        camera_frame = QFrame(self)
        camera_frame.setFrameStyle(QFrame.Box | QFrame.Plain)
        camera_frame.setLineWidth(2)
        camera_layout = QGridLayout(camera_frame)

        # Frame for Speed/Angle display
        speed_angle_frame = QFrame(self)
        speed_angle_frame.setFrameStyle(QFrame.Box | QFrame.Plain)
        speed_angle_frame.setLineWidth(2)
        speed_angle_layout = QGridLayout(speed_angle_frame)

        # Frame for Wheel controls (right side)
        right_frame = QFrame(self)
        right_frame.setFrameStyle(QFrame.Box | QFrame.Plain)
        right_frame.setLineWidth(2)
        right_layout = QGridLayout(right_frame)

        # Robot Camera View Label
        screen_label = QLabel('Robot Camera View', self)
        screen_label.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        screen_label.setStyleSheet("border: 2px solid black; color: black; font-weight: bold;")
        camera_layout.addWidget(screen_label, 0, 0, 1, 2)

        # Speed display label
        self.speed_label = QLabel('Speed: 0', self)
        self.speed_label.setAlignment(Qt.AlignCenter)
        self.speed_label.setStyleSheet("color: black; font-weight: bold; font-size: 24px;")
        speed_angle_layout.addWidget(self.speed_label, 0, 0, 1, 2)

        # Angle display for wheels
        self.wheel_labels = []
        self.wheel_value_labels = []
        for i in range(1, 5):
            angle_label = QLabel(f'Wheel {i} Angle:', self)
            angle_label.setAlignment(Qt.AlignCenter)
            angle_label.setStyleSheet("color: black; font-weight: bold; font-size: 24px;")

            angle_value_label = QLabel('0', self)
            angle_value_label.setAlignment(Qt.AlignCenter)
            angle_value_label.setStyleSheet("color: black; font-weight: bold; font-size: 20px;")

            vbox = QVBoxLayout()
            vbox.addWidget(angle_label)
            vbox.addWidget(angle_value_label)

            if i % 2 == 1:
                row = 1 + (i - 1) // 2
                col = 0
            else:
                row = 1 + (i - 1) // 2
                col = 1
            speed_angle_layout.addLayout(vbox, row, col)

            self.wheel_labels.append((angle_label, angle_value_label))
            self.wheel_value_labels.append(angle_value_label)

        # Wheel controls
        self.add_wheel_control(right_layout, 1, 0, 0, include_speed_buttons=False)  
        self.add_wheel_control(right_layout, 1, 1, 1, include_speed_buttons=False)  
        self.add_wheel_control(right_layout, 2, 0, 2, include_speed_buttons=False)  
        self.add_wheel_control(right_layout, 2, 1, 3, include_speed_buttons=False)  

        # Start and Stop buttons
        start_button = QPushButton('Start', self)
        stop_button = QPushButton('Stop', self)

        # Set button size (width x height)
        start_button.setFixedSize(100, 50) 
        stop_button.setFixedSize(100, 50)

        start_button.clicked.connect(self.start_robot)
        stop_button.clicked.connect(self.stop_robot)

        right_layout.addWidget(start_button, 0, 0, 1, 1)
        right_layout.addWidget(stop_button, 0, 1, 1, 1)

        # Speed control buttons and input field
        button_layout = QGridLayout()
        button_layout.setSpacing(0)
        button_layout.setContentsMargins(0, 0, 0, 0)

        increase_speed_button = QPushButton(self)
        decrease_speed_button = QPushButton(self)

        # Set icons for speed control buttons
        increase_icon_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'doc/increase.PNG'))
        decrease_icon_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'doc/decrease.PNG'))
        
        increase_speed_button.setIcon(QIcon(increase_icon_path))
        decrease_speed_button.setIcon(QIcon(decrease_icon_path))

        # Configure button size and style
        icon_size = QSize(50, 50)
        increase_speed_button.setIconSize(icon_size)
        decrease_speed_button.setIconSize(icon_size)
        increase_speed_button.setFixedSize(50, 50)
        increase_speed_button.setStyleSheet("border: none; margin: 0px; padding: 0px;")
        decrease_speed_button.setFixedSize(50, 50)
        decrease_speed_button.setStyleSheet("border: none; margin: 0px; padding: 0px;")

        # Input field for speed value
        speed_input = QLineEdit(self)
        speed_input.setFixedSize(120, 30)
        speed_input.setStyleSheet("font-size: 16px;")
        speed_input.setAlignment(Qt.AlignCenter)
        speed_input.setPlaceholderText("Speed")
        speed_input.returnPressed.connect(lambda: self.set_speed(speed_input))

        # Link buttons to speed control
        increase_speed_button.clicked.connect(lambda: self.change_all_wheel_speeds(500000))
        decrease_speed_button.clicked.connect(lambda: self.change_all_wheel_speeds(-500000))

        button_layout.addWidget(increase_speed_button, 0, 0)
        button_layout.addWidget(decrease_speed_button, 0, 1)
        button_layout.addWidget(speed_input, 0, 2)

        right_layout.addLayout(button_layout, 3, 0, 1, 2)

        # Connection button
        self.connect_button = QPushButton("Connection", self)
        self.connect_button.clicked.connect(self.show_connection_dialog)
        self.connect_button.setFixedHeight(50)
        right_layout.addWidget(self.connect_button, 4, 0, 1, 2)
        
        # Add Auto button
        self.auto_button = QPushButton("Auto", self)
        self.auto_button.clicked.connect(self.toggle_auto)
        self.auto_button.setStyleSheet("background-color: red;")
        self.auto_button.setFixedHeight(50)
        right_layout.addWidget(self.auto_button, 5, 0, 1, 1)

        # Add Send All Wheel Positions button
        self.send_button = QPushButton("Send All Wheel Angle", self)
        self.send_button.clicked.connect(self.send_all_wheel_positions)
        self.send_button.setFixedSize(200, 50)
        right_layout.addWidget(self.send_button, 5, 1, 1, 1)

        # Add frames to the main layout
        layout.addWidget(camera_frame, 0, 0)
        layout.addWidget(speed_angle_frame, 1, 0)
        layout.addWidget(right_frame, 0, 1, 3, 1)
        layout.setRowStretch(0, 2)  
        layout.setRowStretch(1, 1)
        


        self.show()

    def show_connection_dialog(self):
        dialog = ConnectionDialog(self)
        if dialog.exec_():
            ip, port = dialog.get_connection_info()
            self.connect_to_server(ip, port)

    def connect_to_server(self, ip, port):
        try:
            port = int(port)
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((ip, port))
            print(f"Connected to {ip}:{port}")
            self.connect_button.setText("Connected")
            self.connect_button.setEnabled(False)
            threading.Thread(target=self.receive_data, daemon=True).start()
        except Exception as e:
            print(f"Connection failed: {e}")

    def receive_data(self):
        while True:
            try:
                data = self.socket.recv(1024)
                if not data:
                    break
                message = data.decode()
                self.signals.receivedData.emit(message)
            except:
                break
        print("Disconnected from server")


    def send_command(self, command):
        if self.socket:
            try:
                self.socket.sendall(command.encode())
                print(f"Sent command: {command}")
            except:
                print("Failed to send command")
        else:
            print("Not connected to server")

    def add_wheel_control(self, layout, row, col, wheel_index, include_speed_buttons=True):
        dial = QDial(self)
        dial.setNotchesVisible(True)
        dial.setWrapping(False)  # Disable 360-degree rotation
        dial.setMinimum(-4)  # -200000 / 50000 = -4
        dial.setMaximum(4)   # 200000 / 50000 = 4
        dial.setSingleStep(1)
        dial.setPageStep(1)

        # Connect the dial's value change to update the angle display
        dial.valueChanged.connect(lambda value, idx=wheel_index: self.update_wheel_angle(idx, value * 50000))
        
        # Input field for entering the angle value manually
        angle_input = QLineEdit(self)
        angle_input.setFixedSize(80, 30)
        angle_input.setStyleSheet("font-size: 16px;")
        angle_input.setAlignment(Qt.AlignCenter)
        angle_input.setPlaceholderText("Angle")
        angle_input.returnPressed.connect(lambda idx=wheel_index, input=angle_input: self.set_wheel_angle(idx, input))

        # Layout to hold the dial and the input field
        wheel_layout = QGridLayout()
        wheel_layout.addWidget(dial, 0, 0, Qt.AlignCenter)
        wheel_layout.addWidget(angle_input, 1, 0, Qt.AlignCenter)

        # Add the wheel control layout to the specified position in the main layout
        layout.addLayout(wheel_layout, row, col, Qt.AlignCenter)

    def toggle_auto(self):
        self.is_auto = not self.is_auto
        if self.is_auto:
            self.auto_button.setStyleSheet("background-color: green;")
        else:
            self.auto_button.setStyleSheet("background-color: red;")

    def update_wheel_angle(self, wheel_index, value):
        angle_label, angle_value_label = self.wheel_labels[wheel_index]
        angle_value_label.setText(str(value))

        if self.is_auto:
            # Send command to move the actual robot wheel
            command = f"{wheel_index} tmo {value}\n"
            self.send_command(command)

    def set_wheel_angle(self, wheel_index, input_field):
        try:
            value = int(input_field.text())
            if -200000 <= value <= 200000 and value % 50000 == 0:
                self.wheel_labels[wheel_index][1].setText(str(value))
                input_field.clear()

                # Send command to move the actual robot wheel
                command = f"{wheel_index} tmo {value}\n"
                self.send_command(command)

                # Update dial position
                dial = self.findChild(QDial, f"dial_{wheel_index}")
                if dial:
                    dial.setValue(value // 50000)
            else:
                input_field.setText("Invalid")
        except ValueError:
            input_field.setText("Error")

    def send_all_wheel_positions(self):
        for i in range(4):
            value = int(self.wheel_labels[i][1].text())
            command = f"{i} tmo {value}\n"
            self.send_command(command)

    def change_all_wheel_speeds(self, delta):
        speed_text = self.speed_label.text()
        if ": " in speed_text:
            try:
                current_speed = int(speed_text.split(": ")[1])
                new_speed = current_speed + delta
                self.speed_label.setText(f'Speed: {new_speed}')
            except ValueError:
                pass

    def set_speed(self, input_field):
        try:
            value = int(input_field.text())
            self.speed_label.setText(f'Speed: {value}')
            input_field.clear()
        except ValueError:
            input_field.setText("Error")

    def start_robot(self):
        self.speed_label.setText('Speed: 1500000')
        for angle_label, angle_value_label in self.wheel_labels:
            angle_value_label.setText('0')

    def stop_robot(self):
        self.speed_label.setText('Speed: 0')

    def resizeEvent(self, event):
        self.background.setGeometry(0, 0, self.width(), self.height())
        self.bg_image = QPixmap(self.image_path).scaled(self.width(), self.height(), Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
        self.scene.clear()
        self.scene.addPixmap(self.bg_image)
        super().resizeEvent(event)

    def send_command(self, command):
        if self.socket:
            try:
                
                if not command.endswith('\n'):
                    command += '\n'
                self.socket.sendall(command.encode('utf-8'))
                print(f"Sent command: {command.strip()}")
            except Exception as e:
                print(f"Failed to send command: {e}")
        else:
            print("Not connected to server")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = SkeletonPanel()
    sys.exit(app.exec_())
