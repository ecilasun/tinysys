#pragma once

#include <inttypes.h>

uint32_t MailboxRead(uint32_t hartid, uint32_t address);
void MailboxWrite(uint32_t hartid, uint32_t address, uint32_t value);
