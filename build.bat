@echo off

REM create build directory
pushd %CD%
IF NOT EXIST build mkdir build
cd build

cmake ..

REM USER
set HEAP_SIZE=8388208
set STACK_SIZE= 61800
set PRODUCT= HelloWorld.pdx
set SDK= ..\PlaydateSDK\
set VPATH= ..\
set SRC= ..\main.c
set UINCDIR= 
set UASRC= 
set UDEFS= 
set UADEFS= 
set ULIBDIR=
set ULIBS=

REM DEFAULT
set PDC= %SDK%/bind/pdc

set GCC= arm-none-eabi-gcc.exe
set ASM= arm-none-eabi-gcc.exe -x assembler-with-cpp
set TRGT= arm-none-eabi-

set MCU=cortex-m7
set DDEFS= -DTARGET_PLAYDATE=1 -DTARGET_EXTENSION=1
set DINCDIR= ..\PlaydateSDK\C_API
set DADEFS=
set DLIBDIR=
set DLIBS=
set OPT= -O2 -falign-functions=16 -fomit-frame-pointer
set LDSCRIPT= $(patsubst ~%,$(HOME)%,$(SDK)/C_API/buildsupport/link_map.ld)

set FPU= -mfloat-abi=hard -mfpu=fpv5-sp-d16 -D__FPU_USED=1

set INCDIR= $(patsubst %,-I %,$(DINCDIR) $(UINCDIR))
set LIBDIR= $(patsubst %,-L %,$(DLIBDIR) $(ULIBDIR))
set OBJDIR= build
set DEPDIR= $(OBJDIR)/dep

set DEFS= %DDEFS%

set ADEFS= %DADEFS% %UADEFS% -D__HEAP_SIZE=%HEAP_SIZE% -D__STACK_SIZE=%STACK_SIZE%

set SRC+= $(SDK)/C_API/buildsupport/setup.c

set _OBJS= $(SRC:.c=.o)

set OBJS= $(addprefix $(OBJDIR)/, $(_OBJS))

set LIBS= $(DLIBS) $(ULIBS)
set MCFLAGS= -mthumb -mcpu=%MCU% %FPU%

set ASFLAGS= %MCFLAGS% %OPT% -g3 -gdwarf-2 -Wa, %ADEFS%

set CPFLAGS= %MCFLAGS% %OPT% -gdwarf-2 -Wall -Wno-unused -Wstrict-prototypes -Wno-unknown-pragmas -fverbose-asm -Wdouble-promotion -mword-relocations -fno-common -ffunction-sections -fdata-sections -Wa,-ahlms=%OBJDIR%/main.c %DEFS% -MD -MP

set LDFLAGS= -nostartfiles $(MCFLAGS) -T$(LDSCRIPT) -Wl,-Map=$(OBJDIR)/pdex.map,--cref,--gc-sections,--no-warn-mismatch,--emit-relocs %LIBDIR%

set CLANGFLAGS= -g

REM %PDC% Source %PRODUCT%

REM %GCC% -g3 -c %CPFLAGS% -I %DINCDIR% ../main.c

REM copy build/pdex.elf .
REM copy build/pdex.dylib .

REM %ASM% -c %ASFLAGS% ../main.

