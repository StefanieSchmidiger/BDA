################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/al/dev_al.c \
../Sources/al/dev_iol.c \
../Sources/al/dev_sm.c 

OBJS += \
./Sources/al/dev_al.o \
./Sources/al/dev_iol.o \
./Sources/al/dev_sm.o 

C_DEPS += \
./Sources/al/dev_al.d \
./Sources/al/dev_iol.d \
./Sources/al/dev_sm.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/al/%.o: ../Sources/al/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Static_Code/PDD" -I"C:\Users\Stefanie\Documents\Studium\6.Semester\BDA\Programming\Sources\al" -I"C:\Users\Stefanie\Documents\Studium\6.Semester\BDA\Programming\Sources\dl" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Static_Code/IO_Map" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Sources" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


