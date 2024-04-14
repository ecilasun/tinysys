#pragma once

#include <inttypes.h>

// Set by each HART on boot
#define MAILSLOT_HART_AWAKE		0x00
// Address of this HART's task context
#define MAILSLOT_HART_EXEC		0x01
// Stop given task
#define MAILSLOT_HART_STOP		0x02

uint32_t MailboxRead(uint32_t hartid, uint32_t address);
void MailboxWrite(uint32_t hartid, uint32_t address, uint32_t value);
