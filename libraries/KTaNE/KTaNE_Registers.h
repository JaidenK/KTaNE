#ifndef _KTANE_REGISTERS_H_
#define _KTANE_REGISTERS_H_

#include <stdint.h>

#define REG_DEVID (0x00)
#define DEVID 0xF5

extern volatile uint8_t STATUS;
#define REG_STATUS  (0x10)
#define STS_STRIKE  (0)
#define STS_SOLVED  (1)
#define STS_RUNNING (2)
#define STS_REQUEST (3)
#define STS_READY   (4)
#define STS_FAULT   (5)
#define STS_RESULT_READY (6)

extern volatile uint8_t CONTROL;
#define REG_CTRL      (0x11)
#define CTRL_START    (0)
#define CTRL_RESET    (1)
#define CTRL_LED1     (2)
#define CTRL_LED2     (3)
#define CTRL_DETONATE (4)
#define CTRL_SEFLTEST (5)

extern volatile uint8_t REQUEST;
#define REG_REQUEST (0x12)
#define REQ_DIGITS  (0x01)

// These registers store no value. They're more commands than single-byte registers.
#define REG_REJOIN  (0x13)
#define REG_SYNC    (0x14)

extern volatile uint8_t TEST_RESULTS;
#define REG_TEST_RESULTS (0x15)
#define SELFTEST_NOT_PERFORMED (0)
#define SELFTEST_SUCCESS       (1)
// Additional self test result values
// reserved for module-specific result
// codes

#endif
