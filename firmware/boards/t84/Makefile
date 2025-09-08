# AVR settings
MCU = atmega8a
F_CPU = 8000000UL
CFLAGS = -mmcu=$(MCU) -Wall -Os -DF_CPU=$(F_CPU)

# Files
SRC = main.c
OBJ = $(SRC:.c=.o)
TARGET = firmware

# Raspberry Pi settings
RPI_ADDR = 192.168.0.100
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
	ssh pi@$(RPI_ADDR) sudo $(AVRDUDE) -p m8 -C ./avrdude_gpio.conf -c pi_1 -U flash:w:$(HEX_FILE)
	ssh pi@$(RPI_ADDR) rm $(HEX_FILE)
	$(MAKE) clean

# Cleanup
clean:
	rm -f $(OBJ) $(TARGET).elf $(HEX_FILE)
