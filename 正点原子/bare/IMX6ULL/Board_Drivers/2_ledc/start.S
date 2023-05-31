.global __start

__start:
    /*设置处理器进入 SVC 模式*/
    mrs r0, cpsr @ 将 cpsr 的值读取到 r0
    bic r0, r0, #0x1F @相当于 r0 = r0&(~#0x1f) 把 r0 的最后5位清零
    orr r0, r0, #0x13 @相当于 r0 = r0| (#0x13) 把 r0 的最后5位赋值为 0x13
    msr cpsr, r0 @ 将 r0 写入 cpsr
    /*设置 sp 指针*/
    ldr sp, =0x80200000
    b main @ 跳转程序到c语言的 main 函数
