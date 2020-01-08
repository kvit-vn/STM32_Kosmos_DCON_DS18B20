################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
STM32_Kosmos_DCON_DS18B20.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
STM32_Kosmos_DCON_DS18B20.list \


# All Target
all: STM32_Kosmos_DCON_DS18B20.elf secondary-outputs

# Tool invocations
STM32_Kosmos_DCON_DS18B20.elf: $(OBJS) $(USER_OBJS) C:\Users\kutuk\STM32CubeIDE\workspace_1.1.0\STM32_Kosmos_DCON_DS18B20\STM32F103C8TX_FLASH.ld
	arm-none-eabi-gcc -o "STM32_Kosmos_DCON_DS18B20.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"C:\Users\kutuk\STM32CubeIDE\workspace_1.1.0\STM32_Kosmos_DCON_DS18B20\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="STM32_Kosmos_DCON_DS18B20.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

STM32_Kosmos_DCON_DS18B20.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "STM32_Kosmos_DCON_DS18B20.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
