# -*- coding: utf-8 -*-

import sys
import os
import socket
import threading
from PyQt5.QtWidgets import (QApplication, QWidget, QGridLayout, QVBoxLayout, QHBoxLayout, QLabel, QPushButton, 
                             QDial, QLineEdit, QTextEdit, QMessageBox, QGraphicsView, QGraphicsScene, QFrame, QDialog, 
                             QTextEdit, QMessageBox, QDesktopWidget)
from PyQt5.QtCore import Qt, QSize, pyqtSignal, QObject
from PyQt5.QtGui import QIcon, QPixmap

# Function to center a window on the screen
def center_window(window):
    screen = QDesktopWidget().screenNumber(QDesktopWidget().cursor().pos())
    center_point = QDesktopWidget().screenGeometry(screen).center()
    frame_geometry = window.frameGeometry()
    frame_geometry.moveCenter(center_point)
    window.move(frame_geometry.topLeft())

# Custom signal class for socket communication
class SocketSignals(QObject):
    receivedData = pyqtSignal(str)


#***********************************************    Login Window    **********************************************************# 
# Login window class
class LoginWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        center_window(self)  

    def initUI(self):
        self.setWindowTitle('Login')
        self.setGeometry(300, 300, 400, 250)  # Set window size

        layout = QVBoxLayout()

        # IP input field
        self.ip_input = QLineEdit(self)
        self.ip_input.setPlaceholderText("IP Address")
        self.ip_input.setStyleSheet("font-size: 14px; padding: 5px;")
        layout.addWidget(self.ip_input)

        # Port input field
        self.port_input = QLineEdit(self)
        self.port_input.setPlaceholderText("Port")
        self.port_input.setStyleSheet("font-size: 14px; padding: 5px;")
        layout.addWidget(self.port_input)

        # Connect button
        self.connect_button = QPushButton('Connect', self)
        self.connect_button.clicked.connect(self.try_connect)
        self.connect_button.setStyleSheet("font-size: 14px; padding: 10px;")
        layout.addWidget(self.connect_button)

        # Help button
        self.help_button = QPushButton('Help', self)
        self.help_button.clicked.connect(self.show_help)
        self.help_button.setStyleSheet("font-size: 14px; padding: 10px;")
        layout.addWidget(self.help_button)

        self.setLayout(layout)

    def show_help(self):
        self.help_window = HelpWindow()
        self.help_window.show()

    #connect to IP and PORT address 
    def try_connect(self):
        ip = self.ip_input.text()
        port = self.port_input.text()

        # Check for admin mode
        if ip == "9999" and port == "9999":
            QMessageBox.information(self, "Admin Mode", "Entering admin mode without server connection.")
            self.accept(None)
            return

        try:
            port = int(port)
            socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            socket_obj.connect((ip, port))

            client_type = 0
            socket_obj.sendall(client_type.to_bytes(4, byteorder='little'))

            QMessageBox.information(self, "Success", "Connection successful!")
            self.accept(socket_obj)
        except Exception as e:
            QMessageBox.warning(self, "Error", f"Connection failed: {str(e)}")

    # open main window
    def accept(self, socket_obj):
        self.hide()
        self.main_window = SkeletonPanel(socket_obj)
        self.main_window.show()


#***********************************************    help window    **********************************************************************#

class HelpWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        center_window(self)

    def initUI(self):
        self.setWindowTitle('Help')
        self.setGeometry(300, 300, 500, 400)

        layout = QVBoxLayout()

        help_text = QTextEdit()
        help_text.setReadOnly(True)
        help_text.setHtml("""
        <h1>Robot Controller Help</h1>
        <p>This program allows you to control a robot remotely. Here's how to use it:</p>
        <ol>
            <li><h3>Login:</h3> Enter the IP address and port of the robot server, then click 'Connect'.</li>
            (You can log in in admin mode by entering <strong>9999</strong> in the ip address and port address.)
            <br>
            <li><h3>Main Interface:</h3> Once connected, you'll see the main control panel.</li>
            <br>
            <li><h3>Angle Control:</h3> Turn the dial to adjust the angle of the robot wheel.</li>
            <br>
            <strong>(Auto mode)</strong> The angle of the robot's wheels changes immediately.
            <br>
            <strong>(Non-Auto mode)</strong> If it is not in automatic mode, you must adjust the dial and click the <strong>send all wheel angle</strong> button to change the angle.
            (increasing or decreasing by <strong>45degrees</strong>)
            <br>              
            <li><h3>Speed Control:</h3> Use the acceleration or deceleration buttons to move the robot.</li>
            (Default speed is <strong>10 rpm</strong>, increasing or decreasing by <strong>10 rpm</strong>)
            <br>


            <li><h3>Auto Mode:</h3> Toggle auto mode to control the robot with a keyboard </li>
            <br>
                          
            
            <li><h3>Keyboard controls</h3></li>
                <br><br>
            <strong>Angle Settings</strong>
                <br><br>
            <strong>A</strong>: Turn left (from 0 to -45 degrees (-50000) set)
                <br>
            <strong>S</strong>: Straight (set 0 degree (0))
                <br>    
            <strong>D</strong>: Turn right (from 0 to 45 degrees (50000)
                <br><br>
            <strong>Speed setting</strong>
                <br><br>
            <strong>J</strong>: deceleration (decelerates to the default speed of all wheels by 10 rpm)
                <br>
            <strong>K</strong>: stop (set the speed of all wheels to 0)
                <br>
            <strong>L</strong>: acceleration (the default speed of all wheels is 10 rpm)
                <br><br>

            <strong>Moving forward, backward</strong>
                <br><br>
            <strong>M</strong>: Change the rotation direction of the wheel to select forward or backward</li>
                <br>
            <li><h3>Message Display:</h3> View system messages and command logs in the message area.</li>
        </ol>
        <h2>Made in Line World</h2>
        """)
        layout.addWidget(help_text)

        close_button = QPushButton('Close', self)
        close_button.clicked.connect(self.close)
        layout.addWidget(close_button)

        self.setLayout(layout)


#***********************************************    Main application window    **********************************************************#
# Main application window class
class SkeletonPanel(QWidget):

    # Initialize the main application window
    # socket_obj: Socket object for communication with the server
    def __init__(self, socket_obj=None):
        super().__init__()
        self.socket = socket_obj
        self.is_auto = False
        self.current_speed = 0
        self.angle_change_cmd = "tmo"
        self.speed_change_cmd = "twv"
        self.direction_change_cmd = "wro"
        self.direction_params = "1"
        self.speed_increment = 0x2BB0 # 10RPM (DEC = RPM * 512 * 4096 / 1875)
        self.initUI()
        center_window(self)
        self.signals = SocketSignals()
        self.signals.receivedData.connect(self.log_message)
        if self.socket:
            threading.Thread(target=self.receive_data, daemon=True).start()
        self.update_direction_button_color()  # Initialize M button color

    # Set up the main user interface
    # This includes creating frames for camera view, speed/angle display, and controls
    def initUI(self):
        self.window_width = 1200
        self.window_height = 800
        self.setGeometry(100, 100, self.window_width, self.window_height)
        self.setWindowTitle('Robot Controller')

        # Set background image
        self.image_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'doc/background.jpg'))
        self.background = QGraphicsView(self)
        self.scene = QGraphicsScene()
        self.background.setScene(self.scene)
        self.background.setStyleSheet("background: transparent; border: none;")
        self.background.setGeometry(0, 0, self.window_width, self.window_height)
        self.bg_image = QPixmap(self.image_path).scaled(self.window_width, self.window_height, Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
        self.scene.addPixmap(self.bg_image)

        # Main layout
        layout = QGridLayout()
        self.setLayout(layout)

        # Create and add frames
        self.create_message_frame(layout)
        self.create_speed_angle_frame(layout)
        self.create_right_frame(layout)

        # Set layout properties
        layout.setRowStretch(0, 4)  
        layout.setRowStretch(1, 4)
        layout.setRowStretch(2, 1)
        layout.setColumnStretch(0, 1)
        layout.setColumnStretch(1, 1)

        self.show()


    # code for creating message frame
    def create_message_frame(self, layout):
        # Frame for message
        self.camera_frame = QFrame(self)
        self.camera_frame.setFrameStyle(QFrame.Box | QFrame.Plain)
        self.camera_frame.setLineWidth(2)
        self.camera_frame.setMinimumSize(int(self.window_width * 0.45), int(self.window_height * 0.45))
        camera_layout = QVBoxLayout(self.camera_frame)

        screen_label = QLabel('Message', self)
        screen_label.setAlignment(Qt.AlignLeft | Qt.AlignTop)
        screen_label.setStyleSheet("border: 2px solid black; color: black; font-weight: bold;")
        camera_layout.addWidget(screen_label)

        self.message_display = QTextEdit(self)
        self.message_display.setReadOnly(True)
        self.message_display.setMinimumSize(int(self.window_width * 0.4), int(self.window_height * 0.4))
        camera_layout.addWidget(self.message_display)

        layout.addWidget(self.camera_frame, 0, 0, 2, 1)


    # Create and set up the frame for displaying speed and wheel angles
    def create_speed_angle_frame(self, layout):
        # Frame for Speed/Angle display
        speed_angle_frame = QFrame(self)
        speed_angle_frame.setFrameStyle(QFrame.Box | QFrame.Plain)
        speed_angle_frame.setLineWidth(2)
        speed_angle_layout = QGridLayout(speed_angle_frame)

        # Speed display label
        self.speed_label = QLabel('Speed: 0 (0 RPM)', self)
        self.speed_label.setAlignment(Qt.AlignCenter)
        self.speed_label.setStyleSheet("color: black; font-weight: bold; font-size: 24px;")
        speed_angle_layout.addWidget(self.speed_label, 0, 0, 1, 2)

        # Angle display for wheels
        self.wheel_labels = []
        self.wheel_value_labels = []
        for i in range(1, 5):
            angle_label = QLabel(f'Wheel {i-1} Angle:', self)
            angle_label.setAlignment(Qt.AlignCenter)
            angle_label.setStyleSheet("color: black; font-weight: bold; font-size: 24px;")

            angle_value_label = QLabel('0', self)
            angle_value_label.setAlignment(Qt.AlignCenter)
            angle_value_label.setStyleSheet("color: black; font-weight: bold; font-size: 20px;")

            vbox = QVBoxLayout()
            vbox.addWidget(angle_label)
            vbox.addWidget(angle_value_label)

            row = 1 + (i - 1) // 2
            col = (i - 1) % 2
            speed_angle_layout.addLayout(vbox, row, col)

            self.wheel_labels.append((angle_label, angle_value_label))
            self.wheel_value_labels.append(angle_value_label)

        layout.addWidget(speed_angle_frame, 2, 0)


    # Create and set up the frame for displaying speed and wheel angles
    def create_right_frame(self, layout):
        right_frame = QFrame(self)
        right_frame.setFrameStyle(QFrame.Box | QFrame.Plain)
        right_frame.setLineWidth(2)
        right_layout = QGridLayout(right_frame)

        # Add control buttons
        self.add_control_buttons(right_layout)

        # Add speed control (now below Start and Stop buttons)
        self.add_speed_control(right_layout)

        # Add wheel controls
        wheel_layout = QGridLayout()
        for i in range(4):
            self.add_wheel_control(wheel_layout, i // 2, i % 2, i)
        right_layout.addLayout(wheel_layout, 2, 0, 1, 4)

        # Add keyboard control section
        self.add_keyboard_control(right_layout)

        layout.addWidget(right_frame, 0, 1, 3, 1)


    # Add control buttons (Start, Stop, Auto, Send All Wheel Angle)
    def add_control_buttons(self, layout):
        start_button = QPushButton('Start', self)
        stop_button = QPushButton('Stop', self)
        self.auto_button = QPushButton("Auto", self)
        self.send_button = QPushButton("Send All Wheel Angle", self)

        start_button.setFixedSize(100, 50)
        stop_button.setFixedSize(100, 50)
        self.auto_button.setFixedSize(100, 50)
        self.send_button.setFixedSize(200, 50)

        start_button.clicked.connect(self.start_robot)
        stop_button.clicked.connect(self.stop_robot)
        self.auto_button.clicked.connect(self.toggle_auto)
        self.send_button.clicked.connect(self.send_all_wheel_positions)

        self.auto_button.setStyleSheet("background-color: red;")

        layout.addWidget(start_button, 0, 0, 1, 1)
        layout.addWidget(stop_button, 0, 1, 1, 1)
        layout.addWidget(self.auto_button, 0, 2, 1, 1)
        layout.addWidget(self.send_button, 0, 3, 1, 1)


    # Add speed control elements (increase/decrease buttons and input field)
    def add_speed_control(self, layout):
        speed_layout = QHBoxLayout()

        acceleration_button = QPushButton(self)
        deceleration_button = QPushButton(self)

        acceleration_icon_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'doc/increase.PNG'))
        deceleration_icon_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'doc/decrease.PNG'))
        
        acceleration_button.setIcon(QIcon(acceleration_icon_path))
        deceleration_button.setIcon(QIcon(deceleration_icon_path))

        icon_size = QSize(40, 40)
        acceleration_button.setIconSize(icon_size)
        deceleration_button.setIconSize(icon_size)
        acceleration_button.setFixedSize(40, 40)
        deceleration_button.setFixedSize(40, 40)

        speed_input = QLineEdit(self)
        speed_input.setFixedSize(100, 30)
        speed_input.setStyleSheet("font-size: 14px;")
        speed_input.setAlignment(Qt.AlignCenter)
        speed_input.setPlaceholderText("Speed")
        speed_input.returnPressed.connect(lambda: self.set_speed(speed_input))

        acceleration_button.clicked.connect(lambda: self.change_all_wheel_speeds(self.speed_increment))
        deceleration_button.clicked.connect(lambda: self.change_all_wheel_speeds(-self.speed_increment))

        speed_layout.addWidget(acceleration_button)
        speed_layout.addWidget(deceleration_button)
        speed_layout.addWidget(speed_input)

        layout.addLayout(speed_layout, 1, 0, 1, 4)


    # Add control elements for individual wheels (dial and input field) 
    def add_wheel_control(self, layout, row, col, wheel_index):
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


    # Add keyboard control section with buttons for angle and speed/mode control
    def add_keyboard_control(self, layout):
        keyboard_frame = QFrame(self)
        keyboard_frame.setFrameStyle(QFrame.Box | QFrame.Plain)
        keyboard_layout = QHBoxLayout(keyboard_frame)

        # Angle Control
        angle_control = QVBoxLayout()
        angle_label = QLabel("Angle Control")
        angle_label.setAlignment(Qt.AlignCenter)
        angle_label.setStyleSheet("font-weight: bold;")
        angle_control.addWidget(angle_label)

        angle_buttons = QHBoxLayout()
        self.angle_buttons = {}
        for key in ['A', 'S', 'D']:
            btn = QPushButton(key)
            btn.setFixedSize(40, 40)  # Reduced button size
            angle_buttons.addWidget(btn)
            self.angle_buttons[key] = btn
        angle_control.addLayout(angle_buttons)

        # Speed and Mode Control
        speed_mode_control = QVBoxLayout()
        speed_mode_label = QLabel("Speed and Mode Control")
        speed_mode_label.setAlignment(Qt.AlignCenter)
        speed_mode_label.setStyleSheet("font-weight: bold;")
        speed_mode_control.addWidget(speed_mode_label)

        speed_mode_buttons = QHBoxLayout()
        self.speed_mode_buttons = {}
        for key in ['M', 'J', 'K', 'L']:
            btn = QPushButton(key)
            btn.setFixedSize(40, 40)  # Reduced button size
            speed_mode_buttons.addWidget(btn)
            self.speed_mode_buttons[key] = btn
        speed_mode_control.addLayout(speed_mode_buttons)

        keyboard_layout.addLayout(angle_control)
        keyboard_layout.addLayout(speed_mode_control)

        # Reduce the overall size of the keyboard control section
        keyboard_frame.setFixedHeight(150)  

        layout.addWidget(keyboard_frame, 3, 0, 1, 4)


#***********************************************    Event processing and robot control    **********************************************************#
    # Handle keyboard press events for robot control
    def keyPressEvent(self, event):
        if self.is_auto:
            key = event.text().upper()
            if key in self.angle_buttons:
                self.handle_angle_button_press(key)
            elif key in self.speed_mode_buttons:
                self.handle_speed_mode_button_press(key)

    def update_wheel_angle_display(self, wheel_index, value):
        self.wheel_value_labels[wheel_index].setText(str(value))


    # Handle angle control button presses (A, S, D keys)
    def handle_angle_button_press(self, key):
        if self.is_auto:
            for btn_key, btn in self.angle_buttons.items():
                if btn_key == key:
                    btn.setStyleSheet("background-color: green;")
                    if key == 'A':
                        angle = -50000
                        self.rotate_wheels(angle, angle)
                        self.update_wheel_angle_display(0, angle)
                        self.update_wheel_angle_display(1, angle)
                    elif key == 'D':
                        angle = 50000
                        self.rotate_wheels(angle, angle)
                        self.update_wheel_angle_display(0, angle)
                        self.update_wheel_angle_display(1, angle)
                    elif key == 'S':
                        self.reset_wheels()
                else:
                    btn.setStyleSheet("")
        else:
            self.reset_button_colors()
    
    # Handle speed and moode control button presses (M, J, K, L keys)
    def handle_speed_mode_button_press(self, key):
        if self.is_auto:
            for btn_key, btn in self.speed_mode_buttons.items():
                if btn_key == key:
                    if key != 'M':
                        btn.setStyleSheet("background-color: green;")
                    if key == 'L':
                        self.set_speed_hex(self.current_speed + self.speed_increment)
                    elif key == 'J':
                        self.set_speed_hex(max(0, self.current_speed - self.speed_increment))
                    elif key == 'K':
                        self.set_speed_hex(0)
                    elif key == 'M':
                        self.toggle_direction()
                elif btn_key != 'M':
                    btn.setStyleSheet("")
        else:
            self.reset_button_colors()

    # Toggle the auto mode on/off.
    def toggle_auto(self):
        self.is_auto = not self.is_auto
        self.auto_button.setStyleSheet("background-color: green;" if self.is_auto else "background-color: red;")
        if not self.is_auto:
            self.reset_button_colors()


    def reset_button_colors(self):
        for btn in self.angle_buttons.values():
            btn.setStyleSheet("")
        for key, btn in self.speed_mode_buttons.items():
            if key != 'M':
                btn.setStyleSheet("")


    # Update the wheel angle display and send command to the robot if in auto mode.
    def update_wheel_angle(self, wheel_index, value):
        angle_label, angle_value_label = self.wheel_labels[wheel_index]
        angle_value_label.setText(str(value))

        if self.is_auto:
            # Send command to move the actual robot wheel
            command = f"{wheel_index} {self.angle_change_cmd} {value}\n"
            self.send_command(command)


    # Set the wheel angle based on user input. Validates the input, updates the display, and sends command to the robot.
    def set_wheel_angle(self, wheel_index, input_field):
        try:
            value = int(input_field.text())
            if -200000 <= value <= 200000 and value % 50000 == 0:
                self.wheel_labels[wheel_index][1].setText(str(value))
                input_field.clear()

                # Send command to move the actual robot wheel
                command = f"{wheel_index} {self.angle_change_cmd} {value}\n"
                self.send_command(command)

                # Update dial position
                dial = self.findChild(QDial, f"dial_{wheel_index}")
                if dial:
                    dial.setValue(value // 50000)
            else:
                input_field.setText("Invalid")
        except ValueError:
            input_field.setText("Error")


    # Send the current position of all wheels to the robot.
    def send_all_wheel_positions(self):
        for i in range(4):
            value = int(self.wheel_labels[i][1].text())
            command = f"{i} {self.angle_change_cmd} {value}"
            self.send_command(command)


    # Reset all wheels to the center position (0 degrees).
    def reset_wheels(self):
        self.rotate_wheels(0, 0)
        for i in range(4):
            self.update_wheel_angle_display(i, 0)
        self.log_message("Wheels reset to (0, 0)\n")  


    # Rotate the wheels to the specified angles.
    def rotate_wheels(self, angle0, angle1):
        commands = [
            f"0 {self.angle_change_cmd} {angle0}\n",
            f"1 {self.angle_change_cmd} {angle1}\n"
        ]
        for command in commands:
            self.send_command(command)


    # Toggle the direction of the robot between forward and backward.
    def toggle_direction(self):
        self.direction_params = "0" if self.direction_params == "1" else "1"
        direction_text = "frontward" if self.direction_params == "1" else "backward"
        self.log_message(f"###################")
        self.log_message(f"#Direction Setting: {direction_text}#")
        self.log_message(f"###################\n")
        for i in range(4):
            command = f"{i} {self.direction_change_cmd} {self.direction_params}"
            self.send_command(command)
        self.update_direction_button_color()


    def update_direction_button_color(self):
        if self.direction_params == "1":
            self.speed_mode_buttons['M'].setStyleSheet("background-color: green;")
        else:
            self.speed_mode_buttons['M'].setStyleSheet("background-color: red;")


    def calculate_rpm(self, dec_value):
        return round(dec_value * 1875 / (512 * 4096))


    # Set the speed of all wheels to the specified hex value.
    def set_speed_hex(self, speed_hex):
        self.current_speed = speed_hex
        rpm = self.calculate_rpm(speed_hex)
        self.speed_label.setText(f'Speed: {self.current_speed} ({rpm} RPM)')
        hex_speed = f"0x{self.current_speed:X}"
        for i in range(4):
            command = f"{i} {self.speed_change_cmd} {hex_speed}"
            self.send_command(command)
        self.log_message(f"Speed set to: {hex_speed} ({rpm} RPM)\n")


    # Change the speed of all wheels by the specified delta.
    def change_all_wheel_speeds(self, delta):
        new_speed = self.current_speed + delta
        self.set_speed_hex(max(0, new_speed))


    # Set the speed based on user input.
    def set_speed(self, input_field):
        try:
            value = int(input_field.text())
            self.set_speed_hex(value)
            input_field.clear()
        except ValueError:
            input_field.setText("Error")


    # Start the robot by setting the speed to 1500000 and resetting wheel angles.
    def start_robot(self):
        self.set_speed_hex(self.speed_increment)
        for angle_label, angle_value_label in self.wheel_labels:
            angle_value_label.setText('0')


    # Stop the robot by setting the speed to 0 for all wheels.
    def stop_robot(self):
        self.set_speed_hex(0)


    # Handle the resize event of the window.
    def resizeEvent(self, event):
        self.background.setGeometry(0, 0, self.width(), self.height())
        self.bg_image = QPixmap(self.image_path).scaled(self.width(), self.height(), Qt.IgnoreAspectRatio, Qt.SmoothTransformation)
        self.scene.clear()
        self.scene.addPixmap(self.bg_image)
        super().resizeEvent(event)


#***********************************************    Event processing and robot control    **********************************************************#

    # Receive and process data from the server
    def receive_data(self):
        while self.socket:
            try:
                data = self.socket.recv(1024)
                if not data:
                    break
                message = data.decode()
                self.signals.receivedData.emit(message)
            except Exception as e:
                self.log_message(f"Error receiving data: {e}")
                break
        self.log_message("Disconnected from server")
        self.socket = None

    # Send control commands to the robot
    def send_command(self, command):
        if self.socket:
            try:
                if not command.endswith('\n'):
                    command += '\n'
                self.socket.sendall(command.encode('utf-8'))
                print(f"Sent command: {command.strip()}")
                
                self.log_message(f"Sent command: {command.strip()}")
            except Exception as e:
                print(f"Failed to send command: {e}")
                self.log_message(f"Failed to send command: {e}")
        else:
            print("Not connected to server (Admin Mode)")
            self.log_message("Not connected to server (Admin Mode)")

    
    # Log messages to the UI for user feedback
    def log_message(self, message):
        
        filtered_phrases = [
            "Sent command:",
            "Wheels reset to",
            "Speed set to:",
            "Direction Setting:",
            "#"
        ]
        
        
        if any(phrase in message for phrase in filtered_phrases):
            self.message_display.append(message)
            self.message_display.verticalScrollBar().setValue(
                self.message_display.verticalScrollBar().maximum()
            )
            QApplication.processEvents()

# Main execution
if __name__ == '__main__':
    app = QApplication(sys.argv)
    login_window = LoginWindow()
    login_window.show()
    sys.exit(app.exec_()) 