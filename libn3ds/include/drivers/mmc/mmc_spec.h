#pragma once

/*
 *   This file is part of open_agb_firm
 *   Copyright (C) 2021 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Based on JEDEC eMMC Card Product Standard V4.41.

#include "drivers/toshsd.h"


// TODO: Make a wrapper so this is less controller dependedent?
// TODO: Add required bits/defines for cmds with data payload.


// Basic commands and read-stream command (class 0 and class 1).
#define MMC_GO_IDLE_STATE          (CMD_RESP_NONE |  0u) // Arg [31:0] 0x00000000 GO_IDLE_STATE, 0xF0F0F0F0 GO_PRE_IDLE_STATE, 0xFFFFFFFA BOOT_INITIATION.
#define MMC_SEND_OP_COND           (CMD_RESP_R3   |  1u) // Arg [31:0] OCR with-out busy.
#define MMC_ALL_SEND_CID           (CMD_RESP_R2   |  2u) // Arg [31:0] stuff bits.
#define MMC_SET_RELATIVE_ADDR      (CMD_RESP_R1   |  3u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_SET_DSR                (CMD_RESP_NONE |  4u) // Arg [31:16] DSR [15:0] stuff bits.
#define MMC_SLEEP_AWAKE            (CMD_RESP_R1b  |  5u) // Arg [31:16] RCA [15] Sleep/Awake [14:0] stuff bits.
#define MMC_SWITCH                 (CMD_RESP_R1b  |  6u) // Arg [31:26] Set to 0 [25:24] Access [23:16] Index [15:8] Value [7:3] Set to 0 [2:0] Cmd Set.
#define MMC_SELECT_CARD            (CMD_RESP_R1   |  7u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_DESELECT_CARD          (CMD_RESP_NONE |  7u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_SELECT_CARD_R1b        (CMD_RESP_R1b  |  7u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_SEND_EXT_CSD           (CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 8u) // Arg [31:0] stuff bits.
#define MMC_SEND_CSD               (CMD_RESP_R2   |  9u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_SEND_CID               (CMD_RESP_R2   | 10u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_READ_DAT_UNTIL_STOP    (CMD_MBT | CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 11u) // Arg [31:0] data address.
#define MMC_STOP_TRANSMISSION_R    (CMD_RESP_R1   | 12u) // Arg [31:16] RCA [15:1] stuff bits [0] HPI.
#define MMC_STOP_TRANSMISSION_W    (CMD_RESP_R1b  | 12u) // Arg [31:16] RCA [15:1] stuff bits [0] HPI.
#define MMC_SEND_STATUS            (CMD_RESP_R1   | 13u) // Arg [31:16] RCA [15:1] stuff bits [0] HPI.
#define MMC_BUSTEST_R              (CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 14u) // Arg [31:0] stuff bits.
#define MMC_GO_INACTIVE_STATE      (CMD_RESP_NONE | 15u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_BUSTEST_W              (CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 19u) // Arg [31:0] stuff bits.

// Block-oriented read commands (class 2).
#define MMC_SET_BLOCKLEN           (CMD_RESP_R1   | 16u) // Arg [31:0] block length.
#define MMC_READ_SINGLE_BLOCK      (CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 17u) // Arg [31:0] data address.
#define MMC_READ_MULTIPLE_BLOCK    (CMD_MBT | CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 18u) // Arg [31:0] data address.

// Stream write commands (class 3).
#define MMC_WRITE_DAT_UNTIL_STOP   (CMD_MBT | CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 20u) // Arg [31:0] data address.

// Block-oriented write commands (class 4).
#define MMC_SET_BLOCK_COUNT        (CMD_RESP_R1   | 23u) // Arg [31] Reliable Write Request [30:16] set to 0 [15:0] number of blocks.
#define MMC_WRITE_BLOCK            (CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 24u) // Arg [31:0] data address.
#define MMC_WRITE_MULTIPLE_BLOCK   (CMD_MBT | CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 25u) // Arg [31:0] data address.
#define MMC_PROGRAM_CID            (CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 26u) // Arg [31:0] stuff bits.
#define MMC_PROGRAM_CSD            (CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 27u) // Arg [31:0] stuff bits.

// Block-oriented write protection commands (class 6).
#define MMC_SET_WRITE_PROT         (CMD_RESP_R1b  | 28u) // Arg [31:0] data address.
#define MMC_CLR_WRITE_PROT         (CMD_RESP_R1b  | 29u) // Arg [31:0] data address.
#define MMC_SEND_WRITE_PROT        (CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 30u) // Arg [31:0] write protect data address.
#define MMC_SEND_WRITE_PROT_TYPE   (CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 31u) // Arg [31:0] write protect data address.

// Erase commands (class 5).
#define MMC_ERASE_GROUP_START      (CMD_RESP_R1   | 35u) // Arg [31:0] data address.
#define MMC_ERASE_GROUP_END        (CMD_RESP_R1   | 36u) // Arg [31:0] data address.
#define MMC_ERASE                  (CMD_RESP_R1b  | 38u) // Arg [31] Secure request [30:16] set to 0 [15] Force Garbage Collect request [14:1] set to 0 [0] Identify Write block for Erase.

// I/O mode commands (class 9).
#define MMC_FAST_IO                (CMD_RESP_R4   | 39u) // Arg [31:16] RCA [15:15] register write flag [14:8] register address [7:0] register data.
#define MMC_GO_IRQ_STATE           (CMD_RESP_R5   | 40u) // Arg [31:0] stuff bits.

// Lock card commands (class 7).
#define MMC_LOCK_UNLOCK            (CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 42u) // Arg [31:0] stuff bits.

// Application-specific commands (class 8).
#define MMC_APP_CMD                (CMD_RESP_R1   | 55u) // Arg [31:16] RCA [15:0] stuff bits.
#define MMC_GEN_CMD_R              (CMD_DIR_R | CMD_DT | CMD_RESP_R1 | 56u) // Arg [31:1] stuff bits [0] RD/WR = 1.
#define MMC_GEN_CMD_W              (CMD_DIR_W | CMD_DT | CMD_RESP_R1 | 56u) // Arg [31:1] stuff bits [0] RD/WR = 0.


// MMC R1 card status.
// (S) = status bit.
// (E) = error bit.
// TODO: When do bits get set?
#define MMC_R1_APP_CMD               (1u<<5)  // (S) The card will expect ACMD, or indication that the command has been interpreted as ACMD.
#define MMC_R1_URGENT_BKOPS          (1u<<6)  // (S) If set, device needs to perform backgroundoperations urgently. Host can check EXT_CSD field BKOPS_STATUS for the detailed level.
#define MMC_R1_SWITCH_ERROR          (1u<<7)  // (E) If set, the card did not switch to the expected mode as requested by the SWITCH command.
#define MMC_R1_READY_FOR_DATA        (1u<<8)  // (S) Corresponds to buffer empty signalling on the bus.
#define MMC_R1_STATE_IDLE            (0u<<9)  // (S)
#define MMC_R1_STATE_READY           (1u<<9)  // (S)
#define MMC_R1_STATE_IDENT           (2u<<9)  // (S)
#define MMC_R1_STATE_STBY            (3u<<9)  // (S)
#define MMC_R1_STATE_TRAN            (4u<<9)  // (S)
#define MMC_R1_STATE_DATA            (5u<<9)  // (S)
#define MMC_R1_STATE_RCV             (6u<<9)  // (S)
#define MMC_R1_STATE_PRG             (7u<<9)  // (S)
#define MMC_R1_STATE_DIS             (8u<<9)  // (S)
#define MMC_R1_STATE_BTST            (9u<<9)  // (S)
#define MMC_R1_STATE_SLP             (10u<<9) // (S)
#define MMC_R1_ERASE_RESET           (1u<<13) // (E) An erase sequence was cleared before executing because an out of erase sequence command was received (commands other than CMD35, CMD36, CMD38 or CMD13.
#define MMC_R1_WP_ERASE_SKIP         (1u<<15) // (E) Only partial address space was erased due to existing write protected blocks.
#define MMC_R1_CXD_OVERWRITE         (1u<<16) // (E) Can be either one of the following errors: - The CID register has been already written and can not be overwritten - The read only section of the CSD does not match the card content. - An attempt to reverse the copy (set as original) or permanent WP (unprotected) bits was made.
#define MMC_R1_OVERRUN               (1u<<17) // (E) The card could not sustain data programming in stream write mode.
#define MMC_R1_UNDERRUN              (1u<<18) // (E) The card could not sustain data transfer in stream read mode.
#define MMC_R1_ERROR                 (1u<<19) // (E) (Undefined by the standard) A generic card error related to the (and detected during) execution of the last host command (e.g. read or write failures).
#define MMC_R1_CC_ERROR              (1u<<20) // (E) (Undefined by the standard) A card error occurred, which is not related to the host command.
#define MMC_R1_CARD_ECC_FAILED       (1u<<21) // (E) Card internal ECC was applied but failed to correct the data.
#define MMC_R1_ILLEGAL_COMMAND       (1u<<22) // (E) Command not legal for the card state.
#define MMC_R1_COM_CRC_ERROR         (1u<<23) // (E) The CRC check of the previous command failed.
#define MMC_R1_LOCK_UNLOCK_FAILED    (1u<<24) // (E) Set when a sequence or password error has been detected in lock/unlock card command.
#define MMC_R1_CARD_IS_LOCKED        (1u<<25) // (S) When set, signals that the card is locked by the host.
#define MMC_R1_WP_VIOLATION          (1u<<26) // (E) Attempt to program a write protected block.
#define MMC_R1_ERASE_PARAM           (1u<<27) // (E) An invalid selection of erase groups for erase occurred.
#define MMC_R1_ERASE_SEQ_ERROR       (1u<<28) // (E) An error in the sequence of erase commands occurred.
#define MMC_R1_BLOCK_LEN_ERROR       (1u<<29) // (E) Either the argument of a SET_BLOCKLEN command exceeds the maximum value allowed for the card, or the previously defined block length is illegal for the current command (e.g. the host issues a write command, the current block length is smaller than the card’s maximum and write partial blocks is not allowed).
#define MMC_R1_ADDRESS_MISALIGN      (1u<<30) // (E) The command’ s address argument (in accordance with the currently set block length) positions the first data block misaligned to the card physical blocks. A multiple block read/write operation (although started with a valid address/blocklength combination) is attempting to read or write a data block which does not align with the physical blocks of the card.
#define MMC_R1_ADDRESS_OUT_OF_RANGE  (1u<<31) // (E) The command’s address argument was out of the allowed range for this card. A multiple block or stream read/write operation is (although started in a valid address) attempting to read or write beyond the card capacity.

#define MMC_R1_ERR_ALL               (MMC_R1_ADDRESS_OUT_OF_RANGE | MMC_R1_ADDRESS_MISALIGN | \
                                      MMC_R1_BLOCK_LEN_ERROR | MMC_R1_ERASE_SEQ_ERROR | \
                                      MMC_R1_ERASE_PARAM | MMC_R1_WP_VIOLATION | MMC_R1_LOCK_UNLOCK_FAILED | \
                                      MMC_R1_COM_CRC_ERROR | MMC_R1_ILLEGAL_COMMAND | MMC_R1_CARD_ECC_FAILED | \
                                      MMC_R1_CC_ERROR | MMC_R1_ERROR | MMC_R1_UNDERRUN | MMC_R1_OVERRUN | \
                                      MMC_R1_CXD_OVERWRITE | MMC_R1_WP_ERASE_SKIP | MMC_R1_ERASE_RESET | \
                                      MMC_R1_SWITCH_ERROR)

// Operation Conditions Register (OCR).
// Same bits for CMD1 argument.
#define MMC_OCR_1_7_1_95V            (1u<<7)  // 1.70–1.95V.
/*#define MMC_OCR_2_0_2_1V             (1u<<8)  // 2.0-2.1V.
#define MMC_OCR_2_1_2_2V             (1u<<9)  // 2.1-2.2V.
#define MMC_OCR_2_2_2_3V             (1u<<10) // 2.2-2.3V.
#define MMC_OCR_2_3_2_4V             (1u<<11) // 2.3-2.4V.
#define MMC_OCR_2_4_2_5V             (1u<<12) // 2.4-2.5V.
#define MMC_OCR_2_5_2_6V             (1u<<13) // 2.5-2.6V.
#define MMC_OCR_2_6_2_7V             (1u<<14)*/ // 2.6-2.7V.
#define MMC_OCR_2_7_2_8V             (1u<<15) // 2.7-2.8V.
#define MMC_OCR_2_8_2_9V             (1u<<16) // 2.8-2.9V.
#define MMC_OCR_2_9_3_0V             (1u<<17) // 2.9-3.0V.
#define MMC_OCR_3_0_3_1V             (1u<<18) // 3.0-3.1V.
#define MMC_OCR_3_1_3_2V             (1u<<19) // 3.1-3.2V.
#define MMC_OCR_3_2_3_3V             (1u<<20) // 3.2-3.3V.
#define MMC_OCR_3_3_3_4V             (1u<<21) // 3.3-3.4V.
#define MMC_OCR_3_4_3_5V             (1u<<22) // 3.4-3.5V.
#define MMC_OCR_3_5_3_6V             (1u<<23) // 3.5-3.6V.
#define MMC_OCR_BYTE_MODE            (0u<<29) // Access mode = byte mode.
#define MMC_OCR_SECT_MODE            (2u<<29) // Access mode = sector mode.
#define MMC_OCR_NOT_BUSY             (1u<<31) // Card power up status bit (busy).

// 7.6.1 Command sets and extended settings.
#define MMC_SWITCH_ACC_CMD_SET       (0u)
#define MMC_SWITCH_ACC_SET_BITS      (1u)
#define MMC_SWITCH_ACC_CLR_BITS      (2u)
#define MMC_SWITCH_ACC_WR_BYTE       (3u)
#define MMC_SWITCH_ARG(acc, idx, val, cmdSet)  (((acc)&3u)<<24 | ((idx)&0xFFu)<<16 | ((val)&0xFFu)<<8 | ((cmdSet)&7u))
