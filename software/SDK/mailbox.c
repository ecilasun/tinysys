#include "basesystem.h"
#include "mailbox.h"
#include "core.h"
#include <stdlib.h>

volatile uint32_t *MAILBOX_DEVICE = (volatile uint32_t* ) DEVICE_MAIL;

// Each HART has 64 words of mailbox space

uint32_t MailboxRead(uint32_t hartid, uint32_t address)
{
	return MAILBOX_DEVICE[hartid*64+address];
}

void MailboxWrite(uint32_t hartid, uint32_t address, uint32_t value)
{
	MAILBOX_DEVICE[hartid*64+address] = value;
}