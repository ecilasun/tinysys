#pragma once

#include <inttypes.h>

#define MAILSLOT_HART_AWAKE		0x00
#define MAILSLOT_HART_COMMAND	0x01
#define MAILSLOT_HART_PARAM0	0x02
#define MAILSLOT_HART_PARAM1	0x02
#define MAILSLOT_HART_PARAM2	0x03
#define MAILSLOT_HART_PARAM3	0x04

#define MAIL_CMD_RUN_NOOP		0x00000000
#define MAIL_CMD_RUN_JOB		0x00000001

uint32_t MailboxRead(uint32_t hartid, uint32_t address);
void MailboxWrite(uint32_t hartid, uint32_t address, uint32_t value);
