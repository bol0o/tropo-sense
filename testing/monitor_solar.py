import time
import board
import busio
from adafruit_ina219 import INA219
import csv
from datetime import datetime

# I2C and INA219 setup
i2c = busio.I2C(board.SCL, board.SDA)
ina = INA219(i2c)

# CSV setup
filename = f"solar_log.csv"
with open(filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(["Timestamp", "Avg Current (mA)", "Voltage (V)", "Power (mW)", "Energy (mWh)"])

print(f"Logging started... Saving to {filename}")
total_energy_mWh = 0

try:
    while True:
        readings = []
        start_time = time.time()

        # Take 10 readings per second for 5 seconds
        for _ in range(50):
            try:
                current_mA = ina.current
                if current_mA is not None and current_mA > 0:
                    readings.append(current_mA)
            except Exception:
                pass
            time.sleep(0.1)

        if readings:
            avg_current = sum(readings) / len(readings)
            voltage = ina.bus_voltage 
            power_mW = avg_current * voltage
            energy_mWh = power_mW * (60 / 3600)

            total_energy_mWh += energy_mWh

            timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            print(f"[{timestamp}] Current: {avg_current:.2f} mA | Voltage: {voltage:.2f} V | Power: {power_mW:.2f} mW | Energy: {energy_mWh:.3f} mWh")

            with open(filename, mode='a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow([timestamp, f"{avg_current:.2f}", f"{voltage:.2f}", f"{power_mW:.2f}", f"{energy_mWh:.3f}"])

        else:
            print("No positive current readings detected.")

        # Wait for the rest of the minute
        elapsed = time.time() - start_time
        time.sleep(max(0, 60 - elapsed))

except KeyboardInterrupt:
    print("\nLogging stopped.")
    print(f"Total energy generated: {total_energy_mWh:.2f} mWh = {total_energy_mWh / 1000:.3f}")