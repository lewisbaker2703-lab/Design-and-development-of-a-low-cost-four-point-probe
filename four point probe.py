import serial
import time
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
arduinoSerial = serial.Serial('COM9', 9600, timeout=1)
materials = ["Nail", "Pencil Lead", "Aluminum Can"]
tests_per_material = 3
expected_readings = len(materials) * tests_per_material
readings = []
while True:
    line = arduinoSerial.readline().decode('utf-8').rstrip()
    if not line:
        continue
        print("no data received")
    if line == "DONE":
            break
    data = line.split(",")
    if len(data) != 7:
        print("arduino:",  line)
        continue

    material = materials[int(data[0])]
    test = int(data[1])
    voltage = float(data[2])
    resistance = float(data[3])
    resistivity = float(data[4])
    conductivity = float(data[5])
    temperature = float(data[6])

    reading = {
       "material": material,
       "test": test,
       "voltage": voltage,
       "resistance": resistance,
       "resistivity": resistivity,
       "conductivity": conductivity,
       "temperature": temperature
}
    readings.append(reading)
    print (f"Reading {len(readings)} /{expected_readings}:"
    f"material {material}, test {test}, voltage {voltage:.2f} V, resistance {resistance:.4f} Ohms, resistivity {resistivity:.4f} ohm-m, conductivity {conductivity:.4f} s/m, temperature {temperature:.2f} C")
    print("\nALL DATA RECEIVED")
    print(f"Total readings: {len(readings)}")
df = pd.DataFrame(readings)
print("\nDATA:")
print(df)

comparison = df.groupby("material", sort=False).agg(
    avg_resistance=("resistance", "mean"),
    avg_resistivity=("resistivity", "mean"),
    avg_conductivity=("conductivity", "mean")
).reset_index()

print(comparison)

comparison.plot(kind='bar', x='material', y='avg_resistance', title='Average Resistance by Material')
plt.ylabel("Resistance (Ω)")
plt.tight_layout()
plt.show()

comparison.plot(kind='bar', x='material', y='avg_resistivity', title='Average Resistivity by Material')
plt.ylabel("Resistivity (Ω·m)")
plt.tight_layout()
plt.show()

comparison.plot(kind='bar', x='material', y='avg_conductivity', title='Average Conductivity by Material')
plt.ylabel("Conductivity (S/m)")
plt.tight_layout()
plt.show()

   