################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../BranchConnection.cpp \
../BranchDataSet.cpp \
../BranchField.cpp \
../BranchWriterGroup.cpp \
../InputArguments.cpp \
../Publisher.cpp \
../TreeTrunk.cpp \
../UserInput.cpp \
../main.cpp 

OBJS += \
./BranchConnection.o \
./BranchDataSet.o \
./BranchField.o \
./BranchWriterGroup.o \
./InputArguments.o \
./Publisher.o \
./TreeTrunk.o \
./UserInput.o \
./main.o 

CPP_DEPS += \
./BranchConnection.d \
./BranchDataSet.d \
./BranchField.d \
./BranchWriterGroup.d \
./InputArguments.d \
./Publisher.d \
./TreeTrunk.d \
./UserInput.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


