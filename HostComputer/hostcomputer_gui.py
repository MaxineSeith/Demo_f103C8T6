import sys
import socket
import threading
import time
from collections import deque
from datetime import datetime

from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QLabel, QPushButton, QLineEdit,
                             QFrame, QGraphicsDropShadowEffect, QSlider)
from PyQt5.QtCore import Qt, QTimer, pyqtSignal, QPropertyAnimation, QEasingCurve, QPoint, QRect, pyqtProperty
from PyQt5.QtGui import (QPainter, QColor, QPen, QFont, QLinearGradient,
                        QRadialGradient, QPainterPath, QFontDatabase)


class CyberpunkWidget(QWidget):
    """科幻风格基础Widget"""
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupShadow()

    def setupShadow(self):
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(20)
        shadow.setColor(QColor(0, 255, 255, 100))
        shadow.setOffset(0, 0)
        self.setGraphicsEffect(shadow)


class CircularGauge(QWidget):
    """环形仪表盘"""
    def __init__(self, title, unit, max_value=100, color="#00FFFF", parent=None):
        super().__init__(parent)
        self.title = title
        self.unit = unit
        self.max_value = max_value
        self._current_value = 0
        self.target_value = 0
        self.color = QColor(color)
        self.warning = False

        self.setMinimumSize(200, 200)

        # 动画
        self.animation = QPropertyAnimation(self, b"currentValue")
        self.animation.setDuration(300)
        self.animation.setEasingCurve(QEasingCurve.OutCubic)

    def getCurrentValue(self):
        return self._current_value

    def setCurrentValue(self, value):
        self._current_value = value
        self.update()

    currentValue = pyqtProperty(float, getCurrentValue, setCurrentValue)

    def setValue(self, value):
        self.target_value = value
        self.animation.setStartValue(self._current_value)
        self.animation.setEndValue(value)
        self.animation.start()

    def setWarning(self, warning):
        self.warning = warning
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        width = self.width()
        height = self.height()
        side = min(width, height)

        painter.translate(width / 2, height / 2)
        painter.scale(side / 200.0, side / 200.0)

        # 背景圆环
        pen = QPen(QColor(30, 30, 50, 150))
        pen.setWidth(15)
        pen.setCapStyle(Qt.RoundCap)
        painter.setPen(pen)
        painter.drawArc(-80, -80, 160, 160, 0, 360 * 16)

        # 进度圆环
        if self.warning:
            color = QColor(255, 0, 100)  # 警告红色
        else:
            color = self.color

        gradient = QLinearGradient(-80, -80, 80, 80)
        gradient.setColorAt(0, color)
        gradient.setColorAt(1, color.lighter(150))

        pen = QPen(gradient, 15)
        pen.setCapStyle(Qt.RoundCap)
        painter.setPen(pen)

        angle = int((self._current_value / self.max_value) * 360 * 16)
        painter.drawArc(-80, -80, 160, 160, 90 * 16, -angle)

        # 绘制数值
        painter.setPen(QColor(255, 255, 255))
        font = QFont("Arial", 24, QFont.Bold)
        painter.setFont(font)
        painter.drawText(-50, -10, 100, 40, Qt.AlignCenter, f"{self._current_value:.1f}")

        # 绘制单位
        font.setPointSize(12)
        painter.setFont(font)
        painter.setPen(color)
        painter.drawText(-50, 20, 100, 30, Qt.AlignCenter, self.unit)

        # 绘制标题
        painter.setPen(QColor(150, 150, 200))
        font.setPointSize(10)
        painter.setFont(font)
        painter.drawText(-80, -100, 160, 30, Qt.AlignCenter, self.title)


class WaveformChart(QWidget):
    """波形图"""
    def __init__(self, title, parent=None):
        super().__init__(parent)
        self.title = title
        self.voltage_data = deque(maxlen=100)
        self.current_data = deque(maxlen=100)
        self.setMinimumHeight(200)

    def addData(self, voltage, current):
        self.voltage_data.append(voltage)
        self.current_data.append(current / 100.0)  # 缩放电流
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        width = self.width()
        height = self.height()

        # 背景
        painter.fillRect(0, 0, width, height, QColor(20, 20, 35, 200))

        # 网格
        pen = QPen(QColor(40, 40, 60))
        pen.setStyle(Qt.DotLine)
        painter.setPen(pen)

        for i in range(5):
            y = height * i / 4
            painter.drawLine(0, int(y), width, int(y))

        # 绘制标题
        painter.setPen(QColor(150, 150, 200))
        font = QFont("Arial", 10)
        painter.setFont(font)
        painter.drawText(10, 20, self.title)

        if len(self.voltage_data) < 2:
            return

        # 绘制电压波形
        pen = QPen(QColor(255, 200, 0), 2)
        painter.setPen(pen)

        points = []
        for i, v in enumerate(self.voltage_data):
            x = width * i / 100
            y = height - (v / 10.0) * (height - 40) - 20
            points.append(QPoint(int(x), int(y)))

        if len(points) > 1:
            for i in range(len(points) - 1):
                painter.drawLine(points[i], points[i + 1])

        # 绘制电流波形
        pen = QPen(QColor(0, 255, 200), 2)
        painter.setPen(pen)

        points = []
        for i, c in enumerate(self.current_data):
            x = width * i / 100
            y = height - (c / 5.0) * (height - 40) - 20
            points.append(QPoint(int(x), int(y)))

        if len(points) > 1:
            for i in range(len(points) - 1):
                painter.drawLine(points[i], points[i + 1])

        # 图例
        painter.setPen(QColor(255, 200, 0))
        painter.drawText(width - 150, 20, "— Voltage")
        painter.setPen(QColor(0, 255, 200))
        painter.drawText(width - 150, 40, "— Current")


class CyberpunkButton(QPushButton):
    """科幻风格按钮"""
    def __init__(self, text, color="#00FFFF", parent=None):
        super().__init__(text, parent)
        self.base_color = QColor(color)
        self.is_pressed = False
        self.setupStyle()

    def setupStyle(self):
        r, g, b = self.base_color.red(), self.base_color.green(), self.base_color.blue()
        self.setStyleSheet(f"""
            QPushButton {{
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 rgba({r}, {g}, {b}, 100),
                    stop:1 rgba({r//2}, {g//2}, {b//2}, 100));
                color: white;
                border: 2px solid rgba({r}, {g}, {b}, 200);
                border-radius: 8px;
                padding: 10px 20px;
                font-size: 14px;
                font-weight: bold;
                min-width: 100px;
            }}
            QPushButton:hover {{
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 rgba({r}, {g}, {b}, 150),
                    stop:1 rgba({r//2}, {g//2}, {b//2}, 150));
                border: 2px solid rgba({r}, {g}, {b}, 255);
            }}
            QPushButton:pressed {{
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 rgba({r//2}, {g//2}, {b//2}, 150),
                    stop:1 rgba({r}, {g}, {b}, 150));
            }}
            QPushButton:disabled {{
                background: rgba(50, 50, 50, 100);
                border: 2px solid rgba(80, 80, 80, 150);
                color: rgba(150, 150, 150, 150);
            }}
        """)

        # 阴影效果
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(15)
        shadow.setColor(self.base_color)
        shadow.setOffset(0, 0)
        self.setGraphicsEffect(shadow)


class CyberpunkLineEdit(QLineEdit):
    """科幻风格输入框"""
    def __init__(self, placeholder="", parent=None):
        super().__init__(parent)
        self.setPlaceholderText(placeholder)
        self.setupStyle()

    def setupStyle(self):
        self.setStyleSheet("""
            QLineEdit {
                background: rgba(30, 30, 50, 150);
                color: #00FFFF;
                border: 2px solid rgba(0, 255, 255, 100);
                border-radius: 5px;
                padding: 8px;
                font-size: 14px;
            }
            QLineEdit:focus {
                border: 2px solid rgba(0, 255, 255, 200);
                background: rgba(40, 40, 60, 180);
            }
        """)


class ESPCyberpunkGUI(QMainWindow):
    # 信号
    data_received = pyqtSignal(float, float, float)
    connection_changed = pyqtSignal(bool)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("ESP Monitor - Cyberpunk Edition")
        self.setGeometry(100, 100, 1400, 900)

        # 连接参数
        self.server_ip = "192.168.4.1"
        self.server_port = 8080
        self.client_socket = None
        self.is_connected = False
        self.running = False

        # 数据
        self.voltage_value = 0.0
        self.current_value = 0.0
        self.power_value = 0.0

        # 创建UI
        self.initUI()

        # 连接信号
        self.data_received.connect(self.updateDisplays)
        self.connection_changed.connect(self.onConnectionChanged)

        # 定时器用于更新动画
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.updateAnimations)
        self.timer.start(50)

    def initUI(self):
        # 设置深色背景
        self.setStyleSheet("""
            QMainWindow {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #0a0e27, stop:0.5 #16213e, stop:1 #0f3460);
            }
        """)

        # 主容器
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        main_layout = QVBoxLayout(main_widget)
        main_layout.setSpacing(20)
        main_layout.setContentsMargins(20, 20, 20, 20)

        # 标题栏
        title_label = QLabel("ESP POWER MONITORING SYSTEM")
        title_label.setAlignment(Qt.AlignCenter)
        title_label.setStyleSheet("""
            QLabel {
                color: #00FFFF;
                font-size: 28px;
                font-weight: bold;
                font-family: 'Courier New';
                padding: 20px;
                background: rgba(0, 0, 0, 30);
                border-radius: 10px;
                border: 2px solid rgba(0, 255, 255, 50);
            }
        """)
        main_layout.addWidget(title_label)

        # 连接控制区
        conn_frame = self.createConnectionPanel()
        main_layout.addWidget(conn_frame)

        # 中间内容区 - 分为左右两栏
        content_layout = QHBoxLayout()

        # 左栏 - 仪表盘
        left_panel = self.createGaugePanel()
        content_layout.addWidget(left_panel, 2)

        # 右栏 - 波形图和控制
        right_panel = self.createRightPanel()
        content_layout.addWidget(right_panel, 3)

        main_layout.addLayout(content_layout, 1)

    def createConnectionPanel(self):
        frame = QFrame()
        frame.setStyleSheet("""
            QFrame {
                background: rgba(20, 20, 40, 150);
                border: 2px solid rgba(0, 255, 255, 80);
                border-radius: 10px;
            }
        """)

        layout = QHBoxLayout(frame)
        layout.setContentsMargins(20, 15, 20, 15)

        # 状态指示器
        self.status_label = QLabel("● DISCONNECTED")
        self.status_label.setStyleSheet("""
            QLabel {
                color: #FF0066;
                font-size: 16px;
                font-weight: bold;
                font-family: 'Courier New';
            }
        """)
        layout.addWidget(self.status_label)

        layout.addStretch()

        # 连接按钮
        self.connect_btn = CyberpunkButton("CONNECT", "#00FF88")
        self.connect_btn.clicked.connect(self.connectServer)
        layout.addWidget(self.connect_btn)

        # 断开按钮
        self.disconnect_btn = CyberpunkButton("DISCONNECT", "#FF0066")
        self.disconnect_btn.clicked.connect(self.disconnectServer)
        self.disconnect_btn.setEnabled(False)
        layout.addWidget(self.disconnect_btn)

        return frame

    def createGaugePanel(self):
        frame = QFrame()
        frame.setStyleSheet("""
            QFrame {
                background: rgba(20, 20, 40, 100);
                border: 2px solid rgba(0, 255, 255, 60);
                border-radius: 15px;
            }
        """)

        layout = QVBoxLayout(frame)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(30)

        # 标题
        title = QLabel("REALTIME TELEMETRY")
        title.setAlignment(Qt.AlignCenter)
        title.setStyleSheet("""
            QLabel {
                color: #00FFFF;
                font-size: 18px;
                font-weight: bold;
                font-family: 'Courier New';
                padding: 10px;
            }
        """)
        layout.addWidget(title)

        # 仪表盘
        gauges_layout = QVBoxLayout()
        gauges_layout.setSpacing(20)

        self.voltage_gauge = CircularGauge("VOLTAGE", "V", 10, "#FFD700")
        gauges_layout.addWidget(self.voltage_gauge, alignment=Qt.AlignCenter)

        self.current_gauge = CircularGauge("CURRENT", "mA", 500, "#00FF88")
        gauges_layout.addWidget(self.current_gauge, alignment=Qt.AlignCenter)

        self.power_gauge = CircularGauge("POWER", "W", 5, "#FF00FF")
        gauges_layout.addWidget(self.power_gauge, alignment=Qt.AlignCenter)

        layout.addLayout(gauges_layout)
        layout.addStretch()

        return frame

    def createRightPanel(self):
        frame = QFrame()
        layout = QVBoxLayout(frame)
        layout.setSpacing(20)

        # 波形图
        waveform_frame = QFrame()
        waveform_frame.setStyleSheet("""
            QFrame {
                background: rgba(20, 20, 40, 100);
                border: 2px solid rgba(0, 255, 255, 60);
                border-radius: 15px;
            }
        """)
        waveform_layout = QVBoxLayout(waveform_frame)
        waveform_layout.setContentsMargins(10, 10, 10, 10)

        self.waveform = WaveformChart("POWER WAVEFORM")
        waveform_layout.addWidget(self.waveform)

        layout.addWidget(waveform_frame, 2)

        # 控制面板
        control_panel = self.createControlPanel()
        layout.addWidget(control_panel, 1)

        return frame

    def createControlPanel(self):
        frame = QFrame()
        frame.setStyleSheet("""
            QFrame {
                background: rgba(20, 20, 40, 100);
                border: 2px solid rgba(0, 255, 255, 60);
                border-radius: 15px;
            }
        """)

        layout = QVBoxLayout(frame)
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(15)

        # 标题
        title = QLabel("CONTROL PANEL")
        title.setAlignment(Qt.AlignCenter)
        title.setStyleSheet("""
            QLabel {
                color: #00FFFF;
                font-size: 16px;
                font-weight: bold;
                font-family: 'Courier New';
                padding: 5px;
            }
        """)
        layout.addWidget(title)

        # 电压阈值
        v_layout = QHBoxLayout()
        v_label = QLabel("Voltage Threshold:")
        v_label.setStyleSheet("color: #FFD700; font-size: 14px;")
        v_layout.addWidget(v_label)

        self.voltage_input = CyberpunkLineEdit("Enter voltage (V)")
        v_layout.addWidget(self.voltage_input)

        v_btn = CyberpunkButton("SET", "#FFD700")
        v_btn.clicked.connect(self.sendVoltageMax)
        v_layout.addWidget(v_btn)

        layout.addLayout(v_layout)

        # 电流阈值
        c_layout = QHBoxLayout()
        c_label = QLabel("Current Threshold:")
        c_label.setStyleSheet("color: #00FF88; font-size: 14px;")
        c_layout.addWidget(c_label)

        self.current_input = CyberpunkLineEdit("Enter current (mA)")
        c_layout.addWidget(self.current_input)

        c_btn = CyberpunkButton("SET", "#00FF88")
        c_btn.clicked.connect(self.sendCurrentMax)
        c_layout.addWidget(c_btn)

        layout.addLayout(c_layout)

        # 负载控制
        load_layout = QHBoxLayout()
        load_layout.addStretch()

        self.load_btn = CyberpunkButton("TOGGLE LOAD", "#FF00FF")
        self.load_btn.setMinimumWidth(200)
        self.load_btn.clicked.connect(self.toggleLoad)
        load_layout.addWidget(self.load_btn)

        load_layout.addStretch()
        layout.addLayout(load_layout)

        layout.addStretch()

        return frame

    def connectServer(self):
        try:
            self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.client_socket.settimeout(5)
            self.client_socket.connect((self.server_ip, self.server_port))

            self.is_connected = True
            self.running = True

            # 启动接收线程
            self.receive_thread = threading.Thread(target=self.receiveData, daemon=True)
            self.receive_thread.start()

            self.connection_changed.emit(True)

        except Exception as e:
            print(f"Connection failed: {e}")
            self.is_connected = False

    def disconnectServer(self):
        try:
            self.running = False
            self.is_connected = False

            if self.client_socket:
                self.client_socket.close()
                self.client_socket = None

            self.connection_changed.emit(False)

        except Exception as e:
            print(f"Disconnect error: {e}")

    def receiveData(self):
        buffer = ""
        while self.running and self.is_connected:
            try:
                data = self.client_socket.recv(1024).decode('utf-8', errors='ignore')
                if not data:
                    break

                buffer += data

                while '\n' in buffer:
                    line, buffer = buffer.split('\n', 1)
                    line = line.strip()
                    if line:
                        self.parseData(line)

                if buffer and len(buffer) > 50:
                    self.parseData(buffer)
                    buffer = ""

            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    print(f"Receive error: {e}")
                break

        if self.is_connected:
            self.connection_changed.emit(False)

    def parseData(self, data_str):
        try:
            # 添加调试日志，查看接收到的原始数据
            print(f"[DEBUG] Received data: {repr(data_str)}")

            if 'voltage' in data_str and 'current' in data_str:
                parts = data_str.split()
                print(f"[DEBUG] Split parts: {parts}")

                for i, part in enumerate(parts):
                    if part.startswith('voltage'):
                        voltage_str = part.replace('voltage', '')
                        if voltage_str:
                            self.voltage_value = float(voltage_str) / 10.0
                            print(f"[DEBUG] Parsed voltage (method 1): {self.voltage_value}V")
                    elif i > 0 and parts[i-1] == 'voltage':
                        self.voltage_value = float(part) / 10.0
                        print(f"[DEBUG] Parsed voltage (method 2): {self.voltage_value}V")
                    elif part.startswith('current'):
                        current_str = part.replace('current', '')
                        if current_str:
                            self.current_value = float(current_str)
                            print(f"[DEBUG] Parsed current (method 1): {self.current_value}mA")
                    elif i > 0 and parts[i-1] == 'current':
                        self.current_value = float(part)
                        print(f"[DEBUG] Parsed current (method 2): {self.current_value}mA")

                self.power_value = (self.voltage_value * self.current_value) / 1000.0
                print(f"[DEBUG] Calculated power: {self.power_value}W")
                print(f"[DEBUG] Emitting signal - V:{self.voltage_value}V, I:{self.current_value}mA, P:{self.power_value}W")
                self.data_received.emit(self.voltage_value, self.current_value, self.power_value)

        except Exception as e:
            print(f"Parse error: {e} | Data: {repr(data_str)}")

    def updateDisplays(self, voltage, current, power):
        print(f"[DEBUG] updateDisplays called - V:{voltage}V, I:{current}mA, P:{power}W")
        self.voltage_gauge.setValue(voltage)
        self.current_gauge.setValue(current)
        self.power_gauge.setValue(power)
        self.waveform.addData(voltage, current)
        print(f"[DEBUG] Gauges updated successfully")

    def onConnectionChanged(self, connected):
        if connected:
            self.status_label.setText("● CONNECTED")
            self.status_label.setStyleSheet("""
                QLabel {
                    color: #00FF88;
                    font-size: 16px;
                    font-weight: bold;
                    font-family: 'Courier New';
                }
            """)
            self.connect_btn.setEnabled(False)
            self.disconnect_btn.setEnabled(True)
        else:
            self.status_label.setText("● DISCONNECTED")
            self.status_label.setStyleSheet("""
                QLabel {
                    color: #FF0066;
                    font-size: 16px;
                    font-weight: bold;
                    font-family: 'Courier New';
                }
            """)
            self.connect_btn.setEnabled(True)
            self.disconnect_btn.setEnabled(False)

    def sendVoltageMax(self):
        if not self.is_connected:
            return

        try:
            voltage_max = float(self.voltage_input.text())
            voltage_int = int(voltage_max * 10)
            message = f"volMax:{voltage_int}\r\n"
            self.client_socket.sendall(message.encode('utf-8'))
            time.sleep(0.1)
            print(f"Voltage threshold sent: {voltage_max}V")
        except Exception as e:
            print(f"Send voltage error: {e}")

    def sendCurrentMax(self):
        if not self.is_connected:
            return

        try:
            current_max = int(self.current_input.text())
            message = f"currentMax:{current_max}\r\n"
            self.client_socket.sendall(message.encode('utf-8'))
            time.sleep(0.1)
            print(f"Current threshold sent: {current_max}mA")
        except Exception as e:
            print(f"Send current error: {e}")

    def toggleLoad(self):
        if not self.is_connected:
            return

        try:
            message = "load:\r\n"
            self.client_socket.sendall(message.encode('utf-8'))
            print("Load toggle sent")
        except Exception as e:
            print(f"Toggle load error: {e}")

    def updateAnimations(self):
        # 可以在这里添加更多动画效果
        pass


def main():
    app = QApplication(sys.argv)

    # 设置全局字体
    font = QFont("Arial", 10)
    app.setFont(font)

    window = ESPCyberpunkGUI()
    window.show()

    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
