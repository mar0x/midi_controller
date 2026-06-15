all: firmware
.PHONY: firmware upload init clean download

FQBN = arduino:avr:leonardo
#FQBN = romashka:avr:rocketboard11
BUILD = build

firmware:
	arduino-cli -v compile -b $(FQBN) $(F) --build-path $(BUILD) \
    --build-property "compiler.c.elf.extra_flags=-g" \
    --build-property 'build.usb_manufacturer="Romashka"' \
    --build-property 'build.usb_product="FlightControl"' \
    --libraries ~/.arduino15/libraries \
    --libraries ~/Library/Arduino15/libraries \
    --warnings all .

upload: firmware
	arduino-cli -v upload -b $(FQBN) $(F) --build-path $(BUILD) -p $(PORT) .

CXXFLAGS += -std=c++11

test: BMP280.o MPU9250.o cam_board.o led.o oled.o radio.o rtc.o sensors.o servo.o sim.o test_btn.o twi-std.o trex_game.o debug.o test_serial.o buzzer.o

init:
	arduino-cli -v burn-bootloader -b $(FQBN) $(F) \
    -P avrispmkii

MCU=atmega32u4
BOARD=leonardo
DL_SFX ?= -dl.ihex:i
DL_DIR ?= ./dl/

download:
	avrdude -v -patmega32u4 -cavrispmkII -Pusb -D -n \
    -U flash:r:$(DL_DIR)flash-$(MCU)-$(BOARD)$(DL_SFX) \
    -U efuse:r:$(DL_DIR)efuse-$(MCU)-$(BOARD)$(DL_SFX) \
    -U hfuse:r:$(DL_DIR)hfuse-$(MCU)-$(BOARD)$(DL_SFX) \
    -U lfuse:r:$(DL_DIR)lfuse-$(MCU)-$(BOARD)$(DL_SFX) \
    -U lock:r:$(DL_DIR)lock-$(MCU)-$(BOARD)$(DL_SFX)

clean:
	rm -rf $(BUILD)
	rm -f *.o test
