/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <platform_def.h>

/*
 * ASC driver for ASCII console support.
 * Intentionally implemented in assembly without stack support to
 * match all debug situations.
 *
 * TODO: simplify this to match A-TF panic traces conditions.
 */

#define BOARD_ASC_TXBUFFER_REG      (STIH410_DEBUG_USART_BASE + 0x04)
#define BOARD_ASC_STATUS_REG        (STIH410_DEBUG_USART_BASE + 0x14)

.section .text
.align 5

/*
 * int __asc_xmit(char*) - Transmit a numm terminated string.
 *
 *    R0 is pointer to null-terminated string
 *    Clobbers r0-r3
 */
.global __asc_xmit
__asc_xmit:

    LDR r2, =BOARD_ASC_TXBUFFER_REG
    LDR r3, =BOARD_ASC_STATUS_REG

    /* Output byte */
nextchr:
    /* Spin until TX FIFO ready */
crwait:
    LDR r1, [r3]
    ANDS r1, r1, #0x04    /* AND TX FIFO HALF EMPTY flag */
    BEQ crwait            /* ANDS should have set Z bit if zero */

    LDRB r1, [r0], #1
    ANDS r1, r1, r1
    BEQ  _asc_exit
    CMP r1, #0xa          /* r1 == \n (line feed) ? */
    BNE notlf

    /* Transmit character extra carriage return for each line feed */
    LDR r1, =0x0d
    STR r1, [r2]

    LDR r1, =0x0a         /* replace line feed */

notlf:
    /* Transmit character */
    STR r1, [r2]

    /* Keep going */
    B nextchr
_asc_exit:
    LDR r0, =0
    BX lr

/*
 * void __asc_flush(void) - flush ASC tx fifo.
 *
 *    Clobbers r0-r3
 */
.global __asc_flush
__asc_flush:

    LDR r3, =BOARD_ASC_STATUS_REG

flush_wait:
    LDR r1, [r3]
    ANDS r1, r1, #0x02   /* AND TX FIFO EMPTY flag */
    BEQ flush_wait          /* ANDS should have set Z bit if zero */

_flush_exit:
    LDR r0, =0
    BX lr


/*
 * int __asc_xmit_char(char) - Transmit a single character.
 *
 *    R0 is the 1-byte character to be transmited
 *    Clobbers r0-r3
 */
.global __asc_xmit_char
__asc_xmit_char:

    LDR r2, =BOARD_ASC_TXBUFFER_REG
    LDR r3, =BOARD_ASC_STATUS_REG

    /* Output byte */

    /* Spin until TX FIFO ready */
__asc_char_crwait:
    LDR r1, [r3]
    ANDS r1, r1, #0x04         /* AND TX FIFO HALF EMPTY flag */
    BEQ __asc_char_crwait      /* ANDS should have set Z bit if zero */

    MOVS r1, r0
    LDR r0, =0xFF
    AND r1, r1, r0
    BEQ __asc_char_exit
    CMP r1, #0xa               /* r1 == \n (line feed) ? */
    BNE __asc_char_notlf

    /* Transmit character extra carriage return for each line feed */
    LDR r1, =0x0d
    STR r1, [r2]

    LDR r1, =0x0a              /* replace line feed */

__asc_char_notlf:
    /* Transmit character */
    STR r1, [r2]

__asc_char_exit:
    LDR r0, =0
    BX lr
