import csv
import os
import time
import re
import zipfile
from os.path import isfile
from serial import Serial
import serial.tools.list_ports

# Find the appropriate port (adjust if necessary)
ports = list(serial.tools.list_ports.comports())
port_num = None
for p in ports:
    print(p)
    if "COM" in p.description:
        port_num = p.device
        break

if port_num is None:
    raise RuntimeError("No COM port found.")

esp32 = Serial(port=port_num, baudrate=115200, timeout=0.15)

# Create a unique filename in the format: data_YYYY-MM-DD_test{num}.csv
dir_path = os.path.dirname(os.path.abspath(__file__))
file_base = f"data_{time.strftime('%Y-%m-%d', time.gmtime())}"
file_ext = ".csv"
test_num = 1
while isfile(os.path.join(dir_path, file_base + f"_test{test_num}" + file_ext)):
    test_num += 1
filename = os.path.join(dir_path, file_base + f"_test{test_num}" + file_ext)

try:
    print("Starting data run. Press Ctrl+C to stop.")
    while True:
        data = esp32.readline()
        if data:
            try:
                # Decode and clean the data string
                decoded = data.decode("utf-8").strip()
                # Split on common delimiters
                values = re.split(r'[ ,;]+', decoded)
                print(values)
                # Append a timestamped row to CSV
                with open(filename, "a", newline="") as f:
                    writer = csv.writer(f, delimiter=",")
                    time_ms = time.time_ns() // 10**6
                    millisecs = str(time_ms % 10**3).zfill(3)
                    timestamp = time.strftime("%H:%M:%S", time.gmtime(time_ms // 10**3)) + ":" + millisecs
                    writer.writerow([timestamp] + values)
            except Exception as e:
                print(f"Error processing data: {e}")
except KeyboardInterrupt:
    print(f"\nEnding Datarun: file saved as {filename}")
    # After breaking out, check file size and compress if over 10MB.
    try:
        if os.path.getsize(filename) > 1 * 1024 * 1024:  # 10MB threshold
            sz = os.path.getsize(filename) > 10 * 1024 * 1024
            zip_filename = filename.replace(".csv", ".zip")
            input = input(f"File  size exceeded 10MB. Compress & replace as zip? [y / enter / n] ")
            if input.lower() == "y" or input == "":
                with zipfile.ZipFile(zip_filename, 'w', zipfile.ZIP_DEFLATED) as zipf:
                    zipf.write(filename, os.path.basename(filename))
                os.remove(filename)
                print(f"File compressed and saved as {zip_filename}")
            elif input.lower() == "n":
                print("File not compressed.")
    except Exception as e:
        print(f"Error during compression: {e}")
