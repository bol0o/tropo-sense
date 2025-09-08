# --- AVR settings ---
MCU   = atmega328p
F_CPU = 16000000UL
BAUD  = 115200

# --- Tools ---
CC      = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude

# --- Build dir ---
BUILD   = build

# --- Flags ---
CFLAGS = -mmcu=$(MCU) -Wall -Os -std=gnu11 \
         -DF_CPU=$(F_CPU) -DBAUD=$(BAUD) \
         -I. -I../../communication -I../../peripherals \
         -MMD -MP

# --- Sources (UWAGA: dwa poziomy w górę) ---
SRC_MAIN = main.c
SRC_COMM = ../../communication/uart_isr.c
SRC_PERI = ../../peripherals/gsm_module.c

# --- Objects w build/ ---
OBJ = \
  $(BUILD)/main.o \
  $(BUILD)/uart_isr.o \
  $(BUILD)/gsm_module.o

DEP = $(OBJ:.o=.d)

TARGET   = firmware
ELF_FILE = $(TARGET).elf
HEX_FILE = $(TARGET).hex

# --- Default ---
all: $(HEX_FILE)

# --- Compile rules ---
$(BUILD)/main.o: $(SRC_MAIN)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/uart_isr.o: $(SRC_COMM)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/gsm_module.o: $(SRC_PERI)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# --- Link ---
$(ELF_FILE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# --- Convert to HEX ---
$(HEX_FILE): $(ELF_FILE)
	$(OBJCOPY) -O ihex $< $@

# --- Flash ---
RPI_ADDR  = 192.168.0.148
RESET_PIN = 25
flash: $(HEX_FILE)
	scp $(HEX_FILE) pi@$(RPI_ADDR):/home/pi
	ssh pi@$(RPI_ADDR) sudo $(AVRDUDE) -c linuxspi -P /dev/spidev0.0:/dev/gpiochip0:$(RESET_PIN) -p $(MCU) -U flash:w:$(HEX_FILE)
	ssh pi@$(RPI_ADDR) rm $(HEX_FILE)
	ssh pi@$(RPI_ADDR) pinctrl set $(RESET_PIN) ip
	$(MAKE) clean

# --- Clean ---
clean:
	rm -rf $(BUILD) $(ELF_FILE) $(HEX_FILE)

# --- Auto deps ---
-include $(DEP)
