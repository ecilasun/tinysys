#include "basesystem.h"
#include "mailbox.h"
#include <stdio.h>

int main()
{
    printf("Mailbox check\n");

	// Send something to HART #1
	MailboxWrite(1, 12, 0xABCD0248);

	// Read value at HART #1 mailbox
	uint32_t mailboxvalue = MailboxRead(1, 12);

	printf("Got %x, expected 0xABCD0248\n", mailboxvalue);

    return 0;
}
