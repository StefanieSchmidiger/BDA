################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/dl/dev_dl_ch.c \
../Sources/dl/dev_dl_eh.c \
../Sources/dl/dev_dl_meh.c \
../Sources/dl/dev_dl_mh.c \
../Sources/dl/dev_dl_oh.c \
../Sources/dl/dev_dl_sh.c 

OBJS += \
./Sources/dl/dev_dl_ch.o \
./Sources/dl/dev_dl_eh.o \
./Sources/dl/dev_dl_meh.o \
./Sources/dl/dev_dl_mh.o \
./Sources/dl/dev_dl_oh.o \
./Sources/dl/dev_dl_sh.o 

C_DEPS += \
./Sources/dl/dev_dl_ch.d \
./Sources/dl/dev_dl_eh.d \
./Sources/dl/dev_dl_meh.d \
./Sources/dl/dev_dl_mh.d \
./Sources/dl/dev_dl_oh.d \
./Sources/dl/dev_dl_sh.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/dl/%.o: ../Sources/dl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections  -g3 -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Static_Code/PDD" -I"C:\Users\Stefanie\Documents\Studium\6.Semester\BDA\Programming\Sources\al" -I"C:\Users\Stefanie\Documents\Studium\6.Semester\BDA\Programming\Sources\dl" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Static_Code/IO_Map" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Sources" -I"C:/Users/Stefanie/Documents/Studium/6.Semester/BDA/Programming/Generated_Code" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


