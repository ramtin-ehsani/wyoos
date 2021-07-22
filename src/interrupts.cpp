#include "interrupts.h"


void printf(char* str);



InterruptHandler::InterruptHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
{
    this->InterruptNumber = InterruptNumber;
    this->interruptManager = interruptManager;
    interruptManager->handlers[InterruptNumber] = this;
}

InterruptHandler::~InterruptHandler()
{
    if(interruptManager->handlers[InterruptNumber] == this)
        interruptManager->handlers[InterruptNumber] = 0;
}

uint32_t InterruptHandler::HandleInterrupt(uint32_t esp)
{
    return esp;
}


InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];
InterruptManager* InterruptManager::ActiveInterruptManager = 0;


void InterruptManager::SetInterruptDescriptorTableEntry(uint8_t interrupt,
    uint16_t CodeSegment, void (*handler)(), uint8_t DescriptorPrivilegeLevel, uint8_t DescriptorType)
{
    // address of pointer to code segment (relative to global descriptor table)
    // and address of the handler (relative to segment)
    interruptDescriptorTable[interrupt].handlerAddressLowBits = ((uint32_t) handler) & 0xFFFF;
    interruptDescriptorTable[interrupt].handlerAddressHighBits = (((uint32_t) handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interrupt].gdt_codeSegmentSelector = CodeSegment;

    const uint8_t IDT_DESC_PRESENT = 0x80;
    interruptDescriptorTable[interrupt].access = IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3) << 5) | DescriptorType;
    interruptDescriptorTable[interrupt].reserved = 0;
}

InterruptManager::InterruptManager(GlobalDescriptorTable* gdt)
 : picMasterCommandPort(0x20),
      picMasterDataPort(0x21),
      picSlaveCommandPort(0xA0),
      picSlaveDataPort(0xA1)
{
	uint16_t CodeSegment = gdt->CodeSegmentSelector();

    	const uint8_t IDT_INTERRUPT_GATE = 0xE;
    	for(uint16_t i = 0; i < 256; i++)
    	{
    	handlers[i] = 0;
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
    	}
    	SetInterruptDescriptorTableEntry(0x20, CodeSegment, &HandleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    	SetInterruptDescriptorTableEntry(0x21, CodeSegment, &HandleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
        SetInterruptDescriptorTableEntry(0x2C, CodeSegment, &HandleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);
    	
    	picMasterCommandPort.Write(0x11);
    	picSlaveCommandPort.Write(0x11);
    	
    	picMasterDataPort.Write(0x20);
    	picSlaveDataPort.Write(0x28);
    	
    	picMasterDataPort.Write(0x01);
    	picSlaveDataPort.Write(0x01);

    	picMasterDataPort.Write(0x00);
    	picSlaveDataPort.Write(0x00);
    	
    	InterruptDescriptorTablePointer idt_pointer;
    	idt_pointer.size  = 256*sizeof(GateDescriptor) - 1;
    	idt_pointer.base  = (uint32_t)interruptDescriptorTable;
    	asm volatile("lidt %0" : : "m" (idt_pointer));
}

InterruptManager::~InterruptManager()
{

}

void InterruptManager::Activate()
{
    if(ActiveInterruptManager != 0)
        ActiveInterruptManager->Deactivate();

    ActiveInterruptManager = this;
    asm("sti");
}

void InterruptManager::Deactivate()
{
    if(ActiveInterruptManager == this)
    {
        ActiveInterruptManager = 0;
        asm("cli");
    }
}

uint32_t InterruptManager::handleInterrupt(uint8_t interrupt, uint32_t esp)
{
    if(ActiveInterruptManager != 0)
        return ActiveInterruptManager->DoHandleInterrupt(interrupt, esp);
    //printf(" INTERRUPT");
    return esp;
}

uint32_t InterruptManager::DoHandleInterrupt(uint8_t interrupt, uint32_t esp)
{
    if (handlers[interrupt] != 0)
    {
    	esp = handlers[interrupt]-> HandleInterrupt(esp);
    }
    else if(interrupt != 0x20)
    {
        printf(" INTERRUPT");
    }
    
    
    // hardware interrupts must be acknowledged
    if(0x20 <= interrupt && interrupt < 0x30)
    {
        picMasterCommandPort.Write(0x20);
        if(0x28 <= interrupt)
            picSlaveCommandPort.Write(0x20);
    }


    return esp;
}
