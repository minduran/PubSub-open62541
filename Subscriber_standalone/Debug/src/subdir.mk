################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/pubsub_subscribe_standalone.c \
../src/tutorial_pubsub_publish.c 

OBJS += \
./src/pubsub_subscribe_standalone.o \
./src/tutorial_pubsub_publish.o 

C_DEPS += \
./src/pubsub_subscribe_standalone.d \
./src/tutorial_pubsub_publish.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I/home/minh/open62541/src/pubsub -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


