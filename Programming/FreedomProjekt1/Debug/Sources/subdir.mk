################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/Events.c \
../Sources/dev_appl.c \
../Sources/dev_pl.c \
../Sources/main.c 

OBJS += \
./Sources/Events.o \
./Sources/dev_appl.o \
./Sources/dev_pl.o \
./Sources/main.o 

C_DEPS += \
./Sources/Events.d \
./Sources/dev_appl.d \
./Sources/dev_pl.d \
./Sources/main.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Static_Code/PDD" -I"C:\Users\Stefanie\Documents\Studium\6.Semester\BDA\Programming\Sources\al" -I"C:\Users\Stefanie\Documents\Studium\6.Semester\BDA\Programming\Sources\dl" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Static_Code/IO_Map" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Sources" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


