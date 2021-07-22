#ifndef __INTERRUPTMANAGER_H
#define __INTERRUPTMANAGER_H

#include "gdt.h"
#include "types.h"
#include "port.h"

class InterruptManager;

class InterruptHandler
        {
        protected:
            uint8_t InterruptNumber;
            InterruptManager* interruptManager;
            InterruptHandler(InterruptManager* interruptManager, uint8_t InterruptNumber);
            ~InterruptHandler();
        public:
            virtual uint32_t HandleInterrupt(uint32_t esp);
        };

class InterruptManager
        {
        friend class InterruptHandler;
            protected:
            
             	static InterruptManager* ActiveInterruptManager;
                InterruptHandler* handlers[256];
             	
            	struct GateDescriptor
                {
                    uint16_t handlerAddressLowBits;
                    uint16_t gdt_codeSegmentSelector;
                    uint8_t reserved;
                    uint8_t access;
                    uint16_t handlerAddressHighBits;
                } __attribute__((packed));
                static GateDescriptor interruptDescriptorTable[256];
                
                struct InterruptDescriptorTablePointer
                {
                    uint16_t size;
                    uint32_t base;
                } __attribute__((packed));
                
                static void SetInterruptDescriptorTableEntry(uint8_t interrupt,
                    uint16_t codeSegmentSelectorOffset, void (*handler)(),
                    uint8_t DescriptorPrivilegeLevel, uint8_t DescriptorType);
                    
                Port8BitSlow picMasterCommandPort;
                Port8BitSlow picMasterDataPort;
                Port8BitSlow picSlaveCommandPort;
                Port8BitSlow picSlaveDataPort;
                    
            public:
            	 InterruptManager(GlobalDescriptorTable* gdt);
                ~InterruptManager();
                
                void Activate();
                void Deactivate();
                
                static uint32_t handleInterrupt(uint8_t interrupt, uint32_t esp);
                uint32_t DoHandleInterrupt(uint8_t interrupt, uint32_t esp);
                
                static void IgnoreInterruptRequest();
                
                static void HandleInterruptRequest0x00();
                static void HandleInterruptRequest0x01();
                static void HandleInterruptRequest0x0C();
        };
    
#endif
