################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/r01device/led/GradationControl.cpp \
../source/r01device/led/LED.cpp \
../source/r01device/led/LEDDriver.cpp \
../source/r01device/led/PCA9955B.cpp \
../source/r01device/led/PCA9956B.cpp \
../source/r01device/led/PCA9957.cpp 

CPP_DEPS += \
./source/r01device/led/GradationControl.d \
./source/r01device/led/LED.d \
./source/r01device/led/LEDDriver.d \
./source/r01device/led/PCA9955B.d \
./source/r01device/led/PCA9956B.d \
./source/r01device/led/PCA9957.d 

OBJS += \
./source/r01device/led/GradationControl.o \
./source/r01device/led/LED.o \
./source/r01device/led/LEDDriver.o \
./source/r01device/led/PCA9955B.o \
./source/r01device/led/PCA9956B.o \
./source/r01device/led/PCA9957.o 


# Each subdirectory must supply rules for building sources it contributes
source/r01device/led/%.o: ../source/r01device/led/%.cpp source/r01device/led/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C++ Compiler'
	arm-none-eabi-c++ -std=c++20 -DPRINTF_FLOAT_ENABLE=1 -DSCANF_FLOAT_ENABLE=1 -DPRINTF_ADVANCED_ENABLE=1 -DSCANF_ADVANCED_ENABLE=1 -DCPU_MCXA153VLH -DCPU_MCXA153VLH_cm33_nodsp -DSDK_OS_BAREMETAL -DSERIAL_PORT_TYPE_UART=1 -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__NEWLIB__ -DSDK_DEBUGCONSOLE=0 -DTARGET_A153 -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/source" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/source/r01lib" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/source/r01device" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/board" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/component/lists" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/drivers" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/CMSIS" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/device" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/component/serial_manager" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/utilities" -I"/Users/tedd/dev/mcuxpresso/r01lib_serial/_r01lib_frdm_mcxa153/component/uart" -O2 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fno-rtti -fno-exceptions -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m33+nodsp -mthumb -D__NEWLIB__ -fstack-usage -specs=nano.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-source-2f-r01device-2f-led

clean-source-2f-r01device-2f-led:
	-$(RM) ./source/r01device/led/GradationControl.d ./source/r01device/led/GradationControl.o ./source/r01device/led/LED.d ./source/r01device/led/LED.o ./source/r01device/led/LEDDriver.d ./source/r01device/led/LEDDriver.o ./source/r01device/led/PCA9955B.d ./source/r01device/led/PCA9955B.o ./source/r01device/led/PCA9956B.d ./source/r01device/led/PCA9956B.o ./source/r01device/led/PCA9957.d ./source/r01device/led/PCA9957.o

.PHONY: clean-source-2f-r01device-2f-led

