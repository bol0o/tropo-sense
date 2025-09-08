def in_voltage_range(lookup_table, Vin, target, R1):
    for R2_, _ in lookup_table.items():
        if Vin * (R2_ / (R1 + R2_)) > target:
            return False
        
    return True

def calculate_possible_R1(lookup_table, Vin, target, available_resistors, n):
    posible_values = []
    
    for R2, _ in lookup_table.items():
        R1 = ((Vin * R2) / target) - R2
        
        if in_voltage_range(lookup_table, Vin, target, R1):
            posible_values.append(R1)
            
    distances = []
    for value in posible_values:
        for row in range(len(available_resistors)):
            for resistor_val in available_resistors[row]:
                distances.append((resistor_val, abs(value - resistor_val)))
                
    distances.sort(key=lambda x: x[1])
    
    possible_resistors = []
    for distance in distances:
        if len(possible_resistors) == n:
            break
        if in_voltage_range(lookup_table, Vin, target, distance[0]):
            possible_resistors.append(distance[0])
            
    return possible_resistors

def get_voltage_values(lookup_table, R1):
    for resistance, direction in lookup_table.items():
        output = round(Vin * (resistance / (R1 + resistance)), 2)
        print(f"{{{output}, {direction}}},")

lookup_table = {
    33000: 0,
    6570: 22.5,
    8200: 45,
    891: 67.5,
    1000: 90,
    688: 112.5,
    2200: 135,
    1410: 157.5,
    3900: 180,
    3140: 202.5,
    16000: 225,
    14120: 247.5,
    120000: 270,
    42120: 292.5,
    64900: 315,
    21880: 337.5
}
available_resistors = [
    [1.0, 10, 100, 1.0 * 1000, 10 * 1000, 100 * 1000, 1.0 * 1000000],
    [1.1, 11, 110, 1.1 * 1000, 11 * 1000, 110 * 1000, 1.1 * 1000000],
    [1.2, 12, 120, 1.2 * 1000, 12 * 1000, 120 * 1000, 1.2 * 1000000],
    [1.3, 13, 130, 1.3 * 1000, 13 * 1000, 130 * 1000, 1.3 * 1000000],
    [1.5, 15, 150, 1.5 * 1000, 15 * 1000, 150 * 1000, 1.5 * 1000000],
    [1.6, 16, 160, 1.6 * 1000, 16 * 1000, 160 * 1000, 1.6 * 1000000],
    [1.8, 18, 180, 1.8 * 1000, 18 * 1000, 180 * 1000, 1.8 * 1000000],
    [2.0, 20, 200, 2.0 * 1000, 20 * 1000, 200 * 1000, 2.0 * 1000000],
    [2.2, 22, 220, 2.2 * 1000, 22 * 1000, 220 * 1000, 2.2 * 1000000],
    [2.4, 24, 240, 2.4 * 1000, 24 * 1000, 240 * 1000, 2.4 * 1000000],
    [2.7, 27, 270, 2.7 * 1000, 27 * 1000, 270 * 1000, 2.7 * 1000000],
    [3.0, 30, 300, 3.0 * 1000, 30 * 1000, 300 * 1000, 3.0 * 1000000],
    [3.3, 33, 330, 3.3 * 1000, 33 * 1000, 330 * 1000, 3.3 * 1000000],
    [3.6, 36, 360, 3.6 * 1000, 36 * 1000, 360 * 1000, 3.6 * 1000000],
    [3.9, 39, 390, 3.9 * 1000, 39 * 1000, 390 * 1000, 3.9 * 1000000],
    [4.3, 43, 430, 4.3 * 1000, 43 * 1000, 430 * 1000, 4.3 * 1000000],
    [4.7, 47, 470, 4.7 * 1000, 47 * 1000, 470 * 1000, 4.7 * 1000000],
    [5.1, 51, 510, 5.1 * 1000, 51 * 1000, 510 * 1000, 5.1 * 1000000],
    [5.6, 56, 560, 5.6 * 1000, 56 * 1000, 560 * 1000, 5.6 * 1000000],
    [6.2, 62, 620, 6.2 * 1000, 62 * 1000, 620 * 1000, 6.2 * 1000000],
    [6.8, 68, 680, 6.8 * 1000, 68 * 1000, 680 * 1000, 6.8 * 1000000],
    [7.5, 75, 750, 7.5 * 1000, 75 * 1000, 750 * 1000, 7.5 * 1000000],
    [8.2, 82, 820, 8.2 * 1000, 82 * 1000, 820 * 1000, 8.2 * 1000000],
    [9.1, 91, 910, 9.1 * 1000, 91 * 1000, 910 * 1000, 9.1 * 1000000]
]
Vin = 3.3
target = 2.56

print(calculate_possible_R1(lookup_table, Vin, target, available_resistors, 10))

R1 = 47000
# get_voltage_values(lookup_table, R1)
