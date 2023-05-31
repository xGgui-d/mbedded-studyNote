CROSS_CMOPILE ?= arm-linux-gnueabihf-
TARGET 		  ?= key

CC 			:= $(CROSS_CMOPILE)gcc
LD  		:= $(CROSS_CMOPILE)ld 
OBJCOPY		:= $(CROSS_CMOPILE)objcopy 
OBJDUMP		:= $(CROSS_CMOPILE)objdump 
# 包含头文件的文件夹
INCUDIRS	:= imx6u \
			   bsp/clk \
			   bsp/led \
			   bsp/delay \
			   bsp/beep \
			   bsp/key \
			   bsp/gpio

# 包含源文件的文件夹
SECDIRS		:= project \
			   bsp/clk \
			   bsp/led \
			   bsp/delay \
			   bsp/beep \
			   bsp/key \
			   bsp/gpio

# 将头文件的文件夹的字符串 前面加 -I（用于编译的时候指定头文件的路径）
INCLUDES	:= $(patsubst %,-I %,$(INCUDIRS))

# 根据包含源文件的文件夹中的源文件（包括路径）遍历出来并且赋值给变量
SFILES		:= $(foreach dir, $(SECDIRS), $(wildcard $(dir)/*.S))
CFILES		:= $(foreach dir, $(SECDIRS), $(wildcard $(dir)/*.c))

# 将源文件（包括路径）进行去除路径
SFILENDIR	:= $(notdir $(SFILES))
CFILENDIR	:= $(notdir $(CFILES))

# 将源文件的后缀改为 .o 并且在前面添加 obj/
SOBJS		:= $(patsubst %, obj/%, $(SFILENDIR:.S=.o))
COBJS		:= $(patsubst %, obj/%, $(CFILENDIR:.c=.o))
OBJS		:= $(SOBJS)$(COBJS)

VPATH 		:= $(SECDIRS) 

.PHONY : clean


$(TARGET).bin : $(OBJS)
# 链接文件，采用 imx6u.lds 脚本
	$(LD) -Timx6u.lds -o $(TARGET).elf $^
# 将 .elf 文件转化为 .bin 文件	-S 表示不包含重定位信息和符号信息
	$(OBJCOPY) -O binary -S $(TARGET).elf $@
# 将 .elf 文件进行反汇编，指定格式为 arm -D 表示反汇编所有段	
	$(OBJDUMP) -D -m arm $(TARGET).elf > $(TARGET).dis

# 静态模式，编译多文件
$(SOBJS) : obj/%.o : %.S
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDES) -o $@ $<
$(COBJS) : obj/%.o : %.c
	$(CC) -Wall -nostdlib -c -O2 $(INCLUDES) -o $@ $<

clean :
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).dis $(OBJS)	
