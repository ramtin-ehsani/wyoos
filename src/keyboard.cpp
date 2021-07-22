#include "keyboard.h"

KeyboardDriver::KeyboardDriver(InterruptManager* manager)
: InterruptHandler(manager, 0x21),
dataport(0x60),
commandport(0x64)
{
    while(commandport.Read() & 0x1)
        dataport.Read();
    commandport.Write(0xae); // activate interrupts
    commandport.Write(0x20); // command 0x20 = read controller command byte
    uint8_t status = (dataport.Read() | 1) & ~0x10;
    commandport.Write(0x60); // command 0x60 = set controller command byte
    dataport.Write(status);
    dataport.Write(0xf4);
}

KeyboardDriver::~KeyboardDriver()
{
}

void printf(char* str);
void printfnex(char* str, bool isnext)
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

    for (int x= 0; x < 80; ++x)
    {
        for (int y= 0; y < 25; ++y)
        {
            Videomemory[80*y+x] = (Videomemory[80*y+x] & 0xFFFF);
        }
    }
    
}


uint32_t KeyboardDriver::HandleInterrupt(uint32_t esp)
{
    
    uint8_t key = dataport.Read();
    static bool isShift = false;

    
    switch(key)
    {
    	case 0xFA : break;
    	case 0xC5 : case 0x45 : break; //virtual box stuff not important
    	
    	case 0x02: printf("1"); break;
        case 0x03: printf("2"); break;
        case 0x04: printf("3"); break;
        case 0x05: printf("4"); break;
        case 0x06: printf("5"); break;
        case 0x07: printf("6"); break;
        case 0x08: printf("7"); break;
        case 0x09: printf("8"); break;
        case 0x0A: printf("9"); break;
        case 0x0B: printf("0"); break;

        case 0x10: if(isShift) printfnex("Q",true); else printf("q"); break;
        case 0x11: if(isShift) printfnex("W",false); else printf("w"); break;
        case 0x12: if(isShift) printfnex("E",true); else printf("e"); break;
        case 0x13: if(isShift) printfnex("R",false); else printf("r"); break;
        case 0x14: if(isShift) printfnex("T",true); else printf("t"); break;
        case 0x15: if(isShift) printfnex("Z",false); else printf("z"); break;
        case 0x16: if(isShift) printfnex("U",true); else printf("u"); break;
        case 0x17: if(isShift) printfnex("I",false); else printf("i"); break;
        case 0x18: if(isShift) printfnex("O",true); else printf("o"); break;
        case 0x19: if(isShift) printfnex("P",false); else printf("p"); break;

        case 0x1E: if(isShift) printfnex("A",true); else printf("a"); break;
        case 0x1F: if(isShift) printfnex("S",false); else printf("s"); break;
        case 0x20: if(isShift) printfnex("D",true); else printf("d"); break;
        case 0x21: if(isShift) printfnex("F",false); else printf("f"); break;
        case 0x22: if(isShift) printfnex("G",true); else printf("g"); break;
        case 0x23: if(isShift) printfnex("H",false); else printf("h"); break;
        case 0x24: if(isShift) printfnex("J",true); else printf("j"); break;
        case 0x25: if(isShift) printfnex("K",false); else printf("k"); break;
        case 0x26: if(isShift) printfnex("L",true); else printf("l"); break;

        case 0x2C: if(isShift) printfnex("Y",true); else printf("y"); break;
        case 0x2D: if(isShift) printfnex("X",false); else printf("x"); break;
        case 0x2E: if(isShift) printfnex("C",true); else printf("c"); break;
        case 0x2F: if(isShift) printfnex("V",false); else printf("v"); break;
        case 0x30: if(isShift) printfnex("B",true); else printf("b"); break;
        case 0x31: if(isShift) printfnex("N",false); else printf("n"); break;
        case 0x32: if(isShift) printfnex("M",true); else printf("m"); break;
        case 0x33: printf(","); break;
        case 0x34: printf("."); break;
        case 0x35: printf("-"); break;

        case 0x1C: printf("\n"); break;
        case 0x39: printf(" "); break;

        case 0x2A: case 0x36: isShift = true; break;
        case 0xAA: case 0xB6: isShift = false; break;
    	
    	default:
        if( key< 0x80 ) //not interested in key release
        {
    		char* foo = "KEYBOARD 0x00";
    		char* hex = "0123456789ABCDEF";
    		foo[11] = hex[(key >> 4) & 0x0F];
    		foo[12] = hex[key & 0x0F];
    		printf(foo);
        }
    
    }
    return esp;
}
