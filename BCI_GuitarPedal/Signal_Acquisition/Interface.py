import csv
from os.path import isfile
from serial import Serial
import time
import re
import os

# Port num of esp32 depends on OS and usb slot. Check the Arduino IDE to verify port num on computer. For Mac, use "/dev/cu.usbserial-0001".
import serial.tools.list_ports

ports = list(serial.tools.list_ports.comports())
for p in ports:
    print(p)
    if "COM" in p.description:
        port_num = p.device

esp32 = Serial(port=port_num, baudrate=115200, timeout=0.15)
# Create a filename in the format: res_data_{year}-{month}-{day}_test{num}.
dir = os.path.dirname(os.path.abspath(__file__))
file_base = f"data_{time.strftime('%Y-%m-%d', time.gmtime())}"
file_ext = ".csv"
test_num = 1
while isfile(os.path.join(dir, file_base + f"_test{test_num}" + file_ext)):
    test_num += 1
filename = os.path.join(dir, file_base + f"_test{test_num}" + file_ext)

while True:
    data = esp32.readline()
    if data:
        try:
            decoded_bytes = data[:len(data)-2].decode("utf-8")
            values = re.split(r'[ ,;]', decoded_bytes)
            print(values)
            with open(filename, "a", newline='') as f:
                writer = csv.writer(f, delimiter= ",")
                time_ms = time.time_ns() // (10**6)
                millisecs = str(time_ms % (10**3)).zfill(3)
                writer.writerow([time.strftime("%H:%M:%S", time.gmtime(time_ms // (10**3))) + ":" + millisecs] + values)
        except Exception as e:
            print(f"Error: {e}")
            continue
