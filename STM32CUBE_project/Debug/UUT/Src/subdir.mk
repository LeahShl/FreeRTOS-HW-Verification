################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../UUT/Src/dispatcher.c \
../UUT/Src/hw_verif_crc.c \
../UUT/Src/hw_verif_udp.c \
../UUT/Src/i2c_test.c \
../UUT/Src/uart_test.c 

OBJS += \
./UUT/Src/dispatcher.o \
./UUT/Src/hw_verif_crc.o \
./UUT/Src/hw_verif_udp.o \
./UUT/Src/i2c_test.o \
./UUT/Src/uart_test.o 

C_DEPS += \
./UUT/Src/dispatcher.d \
./UUT/Src/hw_verif_crc.d \
./UUT/Src/hw_verif_udp.d \
./UUT/Src/i2c_test.d \
./UUT/Src/uart_test.d 


# Each subdirectory must supply rules for building sources it contributes
UUT/Src/%.o UUT/Src/%.su UUT/Src/%.cyclo: ../UUT/Src/%.c UUT/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F756xx -c -I../Core/Inc -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I"/home/leah/Documents/RT_Embedded_Course/FreeRTOS-HW-Verification/STM32CUBE_project/UUT/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-UUT-2f-Src

clean-UUT-2f-Src:
	-$(RM) ./UUT/Src/dispatcher.cyclo ./UUT/Src/dispatcher.d ./UUT/Src/dispatcher.o ./UUT/Src/dispatcher.su ./UUT/Src/hw_verif_crc.cyclo ./UUT/Src/hw_verif_crc.d ./UUT/Src/hw_verif_crc.o ./UUT/Src/hw_verif_crc.su ./UUT/Src/hw_verif_udp.cyclo ./UUT/Src/hw_verif_udp.d ./UUT/Src/hw_verif_udp.o ./UUT/Src/hw_verif_udp.su ./UUT/Src/i2c_test.cyclo ./UUT/Src/i2c_test.d ./UUT/Src/i2c_test.o ./UUT/Src/i2c_test.su ./UUT/Src/uart_test.cyclo ./UUT/Src/uart_test.d ./UUT/Src/uart_test.o ./UUT/Src/uart_test.su

.PHONY: clean-UUT-2f-Src

