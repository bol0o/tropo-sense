# AVR settings
MCU = atmega8
F_CPU = 4000000UL
BAUD = 9600
ONE_WIRE_PIN = PB0
ONE_WIRE_PORT = PORTB
ONE_WIRE_DDR = DDRB
ONE_WIRE_PIN_REG = PINB
BME280_ADDR = 0x76
CFLAGS = -mmcu=$(MCU) -Wall -Os -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)

# Files
SRC = main.c i2c.c uart.c ds3231.c
OBJ = $(SRC:.c=.o)
TARGET = firmware

# Raspberry Pi settings
RPI_ADDR = 192.168.18.109
RESET_PIN = 25
HEX_FILE = $(TARGET).hex

# Compiler and tools
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# Default target
all: $(HEX_FILE)

# Compile
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link
$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Convert to HEX
$(HEX_FILE): $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

# Flashing
flash: $(HEX_FILE)
	scp $(HEX_FILE) pi@$(RPI_ADDR):/home/pi
	ssh pi@$(RPI_ADDR) sudo $(AVRDUDE) -c linuxspi -P /dev/spidev0.0:/dev/gpiochip0:$(RESET_PIN) -p m8a -U flash:w:$(HEX_FILE)
	ssh pi@$(RPI_ADDR) rm $(HEX_FILE)
	ssh pi@$(RPI_ADDR) pinctrl set $(RESET_PIN) ip
	$(MAKE) clean

# Cleanup
clean:
	rm -f $(OBJ) $(TARGET).elf $(HEX_FILE)
