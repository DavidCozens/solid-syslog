#ifndef SOLIDSYSLOGSENDERCATEGORIES_H
#define SOLIDSYSLOGSENDERCATEGORIES_H

#include <stdint.h>

#include "SolidSyslogErrorCategory.h"

/*
 * Portable Sender-role error categories. Any Sender implementation (UDP,
 * stream, switching, or an integrator's own) reuses these; a portable handler
 * switch on event->Category works identically across all of them.
 */
#define SOLIDSYSLOG_CAT_SENDER_SEND_NULL_BUFFER ((uint16_t) (SOLIDSYSLOG_CAT_SENDER_BASE + 1U))

#endif /* SOLIDSYSLOGSENDERCATEGORIES_H */
