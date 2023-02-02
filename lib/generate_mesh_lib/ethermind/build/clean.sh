echo off

#REM Root Setting
ROOT=..

echo $ROOT
target=phy6200
target_core=arm_cm0
target_os=phyos
tool_chain=armgcc

make clean -f $ROOT/mesh/build/make/Makefile-ethermind-mesh-core-gcc TOOL_CHAIN=$tool_chain TARGET=$target TARGET_CORE=$target_core TARGET_OS=$target_os
make clean -f $ROOT/utils/build/make/Makefile-ethermind-utils-gcc TOOL_CHAIN=$tool_chain TARGET=$target TARGET_CORE=$target_core TARGET_OS=$target_os
