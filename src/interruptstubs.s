.set IRQ_BASE, 0x20

.section .text

.extern _ZN16InterruptManager15handleInterruptEhj
.global _ZN16InterruptManager22IgnoreInterruptRequestEv

.macro HandleException num
.global _ZN16InterruptManager19HandleException\num\()Ev
_ZN16InterruptManager19HandleException\num\()Ev:
    movb $\num, (interruptnumber)
    jmp int_bottom
.endm


.macro HandleInterruptRequest num
.global _ZN16InterruptManager26HandleInterruptRequest\num\()Ev
_ZN16InterruptManager26HandleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)
    jmp int_bottom
.endm

HandleInterruptRequest 0x00
HandleInterruptRequest 0x01
HandleInterruptRequest 0x0C

int_bottom:
    # save registers
    pusha
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    
    # call C++ Handler
    pushl %esp
    push (interruptnumber)
    call _ZN16InterruptManager15handleInterruptEhj
    #add %esp, 6
    mov %eax, %esp # switch the stack

    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa

_ZN16InterruptManager22IgnoreInterruptRequestEv:
    iret
    
    
    
.data
    interruptnumber: .byte 0
