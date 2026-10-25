"""
build_board.py - Quickshifter ESP32-C3, IGN-cut board (v1).

Generates:
    hardware/quickshifter-c3-ign/quickshifter-c3-ign.kicad_sch

Wiring philosophy (see hardware/README.md for the full rationale):
  * J3 carries the cut ignition-trigger wire. J3.1 and J3.2 are the SAME net
    ("TRIG"), so the ECU keeps driving the coil through the board.
  * Q1 is an ignition IGBT that shorts TRIG to GND on command -> no spark.
  * R7 pulls the IGBT gate to GND, so an unpowered or crashed MCU leaves the
    ignition working normally (fail-safe).
"""

from __future__ import annotations

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from kicadsch import Sch

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(os.path.dirname(HERE), "quickshifter-c3-ign")

TB3 = "TerminalBlock_Phoenix:TerminalBlock_Phoenix_PT-1,5-3-3.5-H_1x03_P3.50mm_Horizontal"

# ref, lib_id, value, footprint, x, y, angle, {pin: net}
PARTS = [
    # ---------------- NGUON / POWER ----------------
    ("J1", "Connector_Generic:Conn_01x03", "BIKE 12V/GND/RPM", TB3, 33, 44, 180,
     {"1": "+12V_IN", "2": "GND", "3": "RPM_IN"}),
    ("F1", "Device:Polyfuse", "1.1A", "Fuse:Fuse_1812_4532Metric", 61, 44, 90,
     {"1": "+12V_IN", "2": "+12V_F"}),
    ("D1", "Device:D_Schottky", "SS34", "Diode_SMD:D_SMA", 86, 44, 180,
     {"2": "+12V_F", "1": "+12V_P"}),
    ("F2", "Device:Polyfuse", "0.5A", "Fuse:Fuse_1206_3216Metric", 112, 44, 90,
     {"1": "+12V_P", "2": "+12V_SENS"}),
    ("D2", "Device:D_TVS", "SMBJ18A", "Diode_SMD:D_SMB", 137, 44, 90,
     {"1": "+12V_P", "2": "GND"}),
    ("C1", "Device:C_Polarized", "100uF/25V", "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm",
     162, 44, 0, {"1": "+12V_P", "2": "GND"}),
    ("C2", "Device:C", "100nF", "Capacitor_SMD:C_0805_2012Metric", 33, 74, 0,
     {"1": "+12V_P", "2": "GND"}),
    ("U1", "Regulator_Switching:LM2596S-5", "LM2596S-5.0",
     "Package_TO_SOT_SMD:TO-263-5_TabPin3", 68, 85, 0,
     {"1": "+12V_P", "2": "SW_NODE", "3": "GND", "4": "+5V", "5": "GND"}),
    ("D3", "Device:D_Schottky", "SS34", "Diode_SMD:D_SMA", 116, 74, 90,
     {"2": "GND", "1": "SW_NODE"}),
    ("L1", "Device:L", "100uH", "Inductor_SMD:L_Bourns_SRN8040TA", 140, 74, 90,
     {"1": "SW_NODE", "2": "+5V"}),
    ("C3", "Device:C_Polarized", "220uF/16V", "Capacitor_THT:CP_Radial_D6.3mm_P2.50mm",
     164, 74, 0, {"1": "+5V", "2": "GND"}),
    ("C4", "Device:C", "100nF", "Capacitor_SMD:C_0805_2012Metric", 33, 104, 0,
     {"1": "+5V", "2": "GND"}),
    ("PF1", "power:PWR_FLAG", "PWR_FLAG", "", 61, 70, 0, {"1": "+12V_P"}),
    ("PF2", "power:PWR_FLAG", "PWR_FLAG", "", 86, 104, 0, {"1": "GND"}),
    ("PF3", "power:PWR_FLAG", "PWR_FLAG", "", 190, 74, 0, {"1": "+5V"}),

    # ---------------- MCU ----------------
    ("U3", "QS:ESP32-C3-SuperMini", "ESP32-C3-SuperMini", "QS:ESP32-C3-SuperMini",
     250, 90, 0,
     {"1": "+5V", "2": "GND", "3": "+3V3",
      "6": "RPM_SIG", "8": "SHIFT_SIG", "10": "IGN_DRV", "12": "TEST_PWM"}),
    ("C8", "Device:C", "100nF", "Capacitor_SMD:C_0805_2012Metric", 205, 118, 0,
     {"1": "+3V3", "2": "GND"}),

    # ---------------- NGÕ VÀO RPM ----------------
    ("R1", "Device:R", "4k7 0.5W",
     "Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal", 33, 165, 90,
     {"1": "RPM_IN", "2": "RPM_LED_A"}),
    ("D4", "Device:D", "1N4148", "Diode_SMD:D_SOD-123", 62, 165, 90,
     {"1": "RPM_LED_A", "2": "GND"}),
    ("U2", "Isolator:PC817", "PC817", "Package_DIP:DIP-4_W7.62mm", 95, 172, 0,
     {"1": "RPM_LED_A", "2": "GND", "3": "GND", "4": "RPM_OPT"}),
    ("R2", "Device:R", "10k", "Resistor_SMD:R_0805_2012Metric", 128, 163, 0,
     {"1": "+3V3", "2": "RPM_OPT"}),
    ("C5", "Device:C", "10nF", "Capacitor_SMD:C_0805_2012Metric", 148, 163, 0,
     {"1": "RPM_OPT", "2": "GND"}),
    ("R3", "Device:R", "1k", "Resistor_SMD:R_0805_2012Metric", 168, 163, 0,
     {"1": "RPM_OPT", "2": "RPM_SIG"}),

    # ---------------- NGÕ VÀO CẦN SỐ ----------------
    ("J2", "Connector_Generic:Conn_01x03", "SN04-N", TB3, 33, 232, 180,
     {"1": "+12V_SENS", "2": "GND", "3": "SHIFT_IN"}),
    ("R4", "Device:R", "4k7", "Resistor_SMD:R_0805_2012Metric", 62, 232, 90,
     {"1": "SHIFT_IN", "2": "SHIFT_SIG"}),
    ("R5", "Device:R", "10k", "Resistor_SMD:R_0805_2012Metric", 90, 222, 0,
     {"1": "+3V3", "2": "SHIFT_SIG"}),
    ("D5", "Device:D_Zener", "BZT52C5V1", "Diode_SMD:D_SOD-123", 113, 222, 0,
     {"1": "SHIFT_SIG", "2": "GND"}),
    ("C6", "Device:C", "10nF", "Capacitor_SMD:C_0805_2012Metric", 136, 222, 0,
     {"1": "SHIFT_SIG", "2": "GND"}),

    # ---------------- CẮT LỬA IGN ----------------
    ("J3", "Connector_Generic:Conn_01x03", "IGN TRIGGER CUT", TB3, 215, 172, 180,
     {"1": "TRIG", "2": "TRIG"}),
    ("R6", "Device:R", "100", "Resistor_SMD:R_0805_2012Metric", 245, 163, 90,
     {"1": "IGN_DRV", "2": "IGBT_G"}),
    ("R7", "Device:R", "10k", "Resistor_SMD:R_0805_2012Metric", 275, 163, 0,
     {"1": "IGBT_G", "2": "GND"}),
    ("D6", "Device:D_Zener", "BZX84C15", "Package_TO_SOT_SMD:SOT-23", 297, 163, 0,
     {"1": "IGBT_G", "2": "GND"}),
    ("C7", "Device:C", "100nF", "Capacitor_SMD:C_0805_2012Metric", 319, 163, 0,
     {"1": "IGBT_G", "2": "GND"}),
    ("Q1", "QS:IGBT_Ignition", "IRGB14C40L",
     "QS:TO-220_Standing", 300, 205, 0,
     {"1": "IGBT_G", "2": "TRIG", "3": "GND"}),

    # ---------------- TEST / DỰ PHÒNG ----------------
    ("R8", "Device:R", "1k", "Resistor_SMD:R_0805_2012Metric", 62, 268, 90,
     {"1": "TEST_PWM", "2": "TEST_PWM_OUT"}),
    ("J4", "Connector_Generic:Conn_01x02",
     "TEST_PWM", "Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical",
     90, 268, 180, {"1": "TEST_PWM_OUT", "2": "GND"}),
]

NO_CONNECT = [("U3", "4"), ("U3", "5"), ("U3", "7"), ("U3", "9"), ("U3", "11"),
              ("U3", "13"), ("U3", "14"), ("U3", "15"), ("U3", "16"), ("J3", "3")]

DNP = {"D6"}

NOTES = [
    ("NGUON / POWER: 12V -> fuse -> chong nguoc -> TVS -> buck LM2596 -> 5V", (25, 28)),
    ("MCU: ESP32-C3 Super Mini (GPIO2=RPM, GPIO4=SHIFT, GPIO6=IGN, GPIO8=TEST_PWM)", (205, 60)),
    ("NGÕ VÀO RPM - cach ly quang PC817", (25, 152)),
    ("NGÕ VÀO CẢM BIẾN CẦN SỐ SN04-N (NPN)", (25, 212)),
    ("CẮT LỬA IGN - IGBT + fail-safe pull-down", (205, 152)),
    ("J3.1 va J3.2 LA CUNG MOT NET (TRIG): cat day trigger, van 2 dau day vao J3.1/J3.2.", (205, 248)),
    ("Khi cat: Q1 noi tat TRIG xuong mass -> mat lua. Mat nguon/MCU treo: R7 keo gate xuong -> IGBT tat -> xe no binh thuong.", (205, 258)),
    ("D6 (zener 15V G-E) mac dinh KHONG dan (DNP) vi IRGB14C40L da co zener bao ve noi bo.", (205, 268)),
]


def main():
    sch = Sch(title="Quickshifter ESP32-C3 - IGN cut board", rev="v1.0",
              comments=["Redrawn in KiCad from the hand sketch. IGN channel only.",
                        "GPIO2=RPM  GPIO4=SHIFT  GPIO6=IGN  GPIO8=TEST_PWM"])

    for ref, lib, value, fp, x, y, ang, nets in PARTS:
        sch.place(lib, ref, value, fp, x, y, ang, dnp=(ref in DNP))
        for pinno, net in nets.items():
            sch.stub(ref, pinno, net)

    for ref, pinno in NO_CONNECT:
        sch.no_connect(ref, pinno)

    for s, pt in NOTES:
        sch.text(s, pt)

    os.makedirs(OUT_DIR, exist_ok=True)
    path = os.path.join(OUT_DIR, "quickshifter-c3-ign.kicad_sch")
    sch.emit(path)
    print("wrote", path)
    print("components:", len(sch.instances), " wires:", len(sch.wires),
          " labels:", len(sch.labels), " no_connects:", len(sch.no_connects))


if __name__ == "__main__":
    main()
