#include "device.h"
#include "x86.h"

extern int displayRow;
extern int displayCol;

void GProtectFaultHandle(struct StackFrame *sf);

void syscallHandle(struct StackFrame *sf);

void syscallWrite(struct StackFrame *sf);
void syscallPrint(struct StackFrame *sf);

void irqHandle(struct StackFrame *sf) {  // pointer sf = esp
    /* Reassign segment register */
    asm volatile("movw %%ax, %%ds" ::"a"(KSEL(SEG_KDATA)));
    /* Save esp to stackTop */
    uint32_t tmpStackTop = pcb[current].stackTop;
    pcb[current].prevStackTop = pcb[current].stackTop;
    pcb[current].stackTop = (uint32_t)sf;

    switch (sf->irq) {
        case -1:
            break;
        case 0xd:
            GProtectFaultHandle(sf);
            break;
        case 0x20:
            timerHandle(sf);
            break;
        case 0x80:
            syscallHandle(sf);
            break;
        default:
            assert(0);
    }

    /* Recover stackTop */
    pcb[current].stackTop = tmpStackTop;
}

void GProtectFaultHandle(struct StackFrame *sf) {
    assert(0);
    return;
}

void syscallHandle(struct StackFrame *sf) {
    switch (sf->eax) {  // syscall number
        case 0:
            syscallWrite(sf);
            break;  // for SYS_WRITE (0)
        /*TODO Add Fork,Sleep... */
        default:
            break;
    }
}

void timerHandle(struct StackFrame *sf) {
    // TODO in lab3
}

void syscallWrite(struct StackFrame *sf) {
    switch (sf->ecx) {  // file descriptor
        case 0:
            syscallPrint(sf);
            break;  // for STD_OUT
        default:
            break;
    }
}

// Attention:
// This is optional homework, because now our kernel can not deal with
// consistency problem in syscallPrint. If you want to handle it, complete this
// function. But if you're not interested in it, don't change anything about it
void syscallPrint(struct StackFrame *sf) {
    int sel = sf->ds;  // TODO segment selector for user data, need further
                       // modification
    char *str = (char *)sf->edx;
    int size = sf->ebx;
    int i = 0;
    int pos = 0;
    char character = 0;
    uint16_t data = 0;
    asm volatile("movw %0, %%es" ::"m"(sel));
    for (i = 0; i < size; i++) {
        asm volatile("movb %%es:(%1), %0" : "=r"(character) : "r"(str + i));
        if (character == '\n') {
            displayRow++;
            displayCol = 0;
            if (displayRow == 25) {
                displayRow = 24;
                displayCol = 0;
                scrollScreen();
            }
        } else {
            data = character | (0x0c << 8);
            pos = (80 * displayRow + displayCol) * 2;
            asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
            displayCol++;
            if (displayCol == 80) {
                displayRow++;
                displayCol = 0;
                if (displayRow == 25) {
                    displayRow = 24;
                    displayCol = 0;
                    scrollScreen();
                }
            }
        }
        // asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
        // asm volatile("int $0x20":::"memory"); //XXX Testing irqTimer during
        // syscall
    }

    updateCursor(displayRow, displayCol);
    // TODO take care of return value
    return;
}

void syscallFork(struct StackFrame *sf) {
    // TODO in lab3
}

void syscallSleep(struct StackFrame *sf) {
    // TODO in lab3
}

void syscallExit(struct StackFrame *sf) {
    // TODO in lab3
}
