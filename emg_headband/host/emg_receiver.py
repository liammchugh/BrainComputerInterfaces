#!/usr/bin/env python3
import asyncio, struct, csv, time
from bleak import BleakScanner, BleakClient

DATA_UUID = "3f5b0002-d946-4844-b1ce-b29134ddeaf5"

async def main():
    dev = await BleakScanner.find_device_by_filter(lambda d, _: "EMG_HEADBAND" in d.name)
    if not dev:
        print("Device not found"); return

    async with BleakClient(dev) as c, open("emg_log.csv", "w", newline="") as f:
        writer = csv.writer(f)
        t0 = time.time()

        def cb(handle, data):
            now = time.time() - t0
            samples = struct.unpack("<" + "h"*5, data)  # 5 channels
            writer.writerow([now, *samples])

        await c.start_notify(DATA_UUID, cb)
        print("Logging… Ctrl-C to stop")
        while True: await asyncio.sleep(1)

asyncio.run(main())
