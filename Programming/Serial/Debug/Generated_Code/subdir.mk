################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Generated_Code/ASerialLdd1.c \
../Generated_Code/BitIoLdd1.c \
../Generated_Code/BitIoLdd2.c \
../Generated_Code/BitIoLdd3.c \
../Generated_Code/Cpu.c \
../Generated_Code/I2C.c \
../Generated_Code/IntI2cLdd1.c \
../Generated_Code/LEDpin1.c \
../Generated_Code/LEDpin2.c \
../Generated_Code/LEDpin3.c \
../Generated_Code/LedBlue.c \
../Generated_Code/LedGreen.c \
../Generated_Code/LedRed.c \
../Generated_Code/PE_LDD.c \
../Generated_Code/SMasterLdd1.c \
../Generated_Code/SPI.c \
../Generated_Code/TI1.c \
../Generated_Code/TU1.c \
../Generated_Code/TimerIntLdd1.c \
../Generated_Code/UART.c \
../Generated_Code/Vectors.c \
../Generated_Code/WAIT1.c 

OBJS += \
./Generated_Code/ASerialLdd1.o \
./Generated_Code/BitIoLdd1.o \
./Generated_Code/BitIoLdd2.o \
./Generated_Code/BitIoLdd3.o \
./Generated_Code/Cpu.o \
./Generated_Code/I2C.o \
./Generated_Code/IntI2cLdd1.o \
./Generated_Code/LEDpin1.o \
./Generated_Code/LEDpin2.o \
./Generated_Code/LEDpin3.o \
./Generated_Code/LedBlue.o \
./Generated_Code/LedGreen.o \
./Generated_Code/LedRed.o \
./Generated_Code/PE_LDD.o \
./Generated_Code/SMasterLdd1.o \
./Generated_Code/SPI.o \
./Generated_Code/TI1.o \
./Generated_Code/TU1.o \
./Generated_Code/TimerIntLdd1.o \
./Generated_Code/UART.o \
./Generated_Code/Vectors.o \
./Generated_Code/WAIT1.o 

C_DEPS += \
./Generated_Code/ASerialLdd1.d \
./Generated_Code/BitIoLdd1.d \
./Generated_Code/BitIoLdd2.d \
./Generated_Code/BitIoLdd3.d \
./Generated_Code/Cpu.d \
./Generated_Code/I2C.d \
./Generated_Code/IntI2cLdd1.d \
./Generated_Code/LEDpin1.d \
./Generated_Code/LEDpin2.d \
./Generated_Code/LEDpin3.d \
./Generated_Code/LedBlue.d \
./Generated_Code/LedGreen.d \
./Generated_Code/LedRed.d \
./Generated_Code/PE_LDD.d \
./Generated_Code/SMasterLdd1.d \
./Generated_Code/SPI.d \
./Generated_Code/TI1.d \
./Generated_Code/TU1.d \
./Generated_Code/TimerIntLdd1.d \
./Generated_Code/UART.d \
./Generated_Code/Vectors.d \
./Generated_Code/WAIT1.d 


# Each subdirectory must supply rules for building sources it contributes
Generated_Code/%.o: ../Generated_Code/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Serial/Static_Code/PDD" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Serial/Static_Code/IO_Map" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Serial/Sources" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Serial/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


