#include "types.h"
#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "mouse.h"

bool isnext = false;

void printf(char* str)
{
    static uint16_t* Videomemory = (uint16_t*)0xb8000;
    static uint8_t x=0,y=0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                if (isnext)
                {
                    Videomemory[80*y+x] = (Videomemory[80*y+x] & 0x0000) | str[i];
                    isnext = false;
                }
                else{
                    isnext = true;
                    Videomemory[80*y+x] = (Videomemory[80*y+x] & 0xFF00) | str[i];
                }
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    Videomemory[80*y+x] = (Videomemory[80*y+x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
    
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(void* multiboot_structure, uint32_t magicnumber)
{
    printf("Hello World!!\n");
    printf("NewLine feature added!");
    
    GlobalDescriptorTable gdt;
    InterruptManager interrupts(&gdt);
    
    KeyboardDriver keyboard(&interrupts);
    MouseDriver mouse(&interrupts);
    
    interrupts.Activate();

    while(1);
    
}
