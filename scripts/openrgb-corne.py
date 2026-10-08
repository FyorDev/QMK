#!/usr/bin/env python3
"""Register the Aurora Corne as a QMK OpenRGB device in the OpenRGB server config.

Run as root: the service config is root-owned and rewritten when the service stops.
"""

import json
import os
import shutil
import subprocess
import sys

CONFIG = "/var/lib/OpenRGB/OpenRGB.json"
DEVICE = {"name": "Aurora Corne rev1", "usb_vid": "8D1D", "usb_pid": "343A"}


def main():
    if os.geteuid() != 0:
        sys.exit("run as root")

    subprocess.run(["systemctl", "stop", "openrgb"], check=True)
    try:
        shutil.copy(CONFIG, CONFIG + ".bak")
        with open(CONFIG) as f:
            config = json.load(f)

        devices = config.setdefault("QMKOpenRGBDevices", {}).setdefault("devices", [])
        if DEVICE not in devices:
            devices.append(DEVICE)

        with open(CONFIG, "w") as f:
            json.dump(config, f, indent=4)
    finally:
        subprocess.run(["systemctl", "start", "openrgb"], check=True)


if __name__ == "__main__":
    main()
