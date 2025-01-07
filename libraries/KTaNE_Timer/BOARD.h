#ifndef BOARD_H
#define	BOARD_H

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

// Boolean defines for TRUE, FALSE, SUCCESS and ERROR
//#ifndef FALSE
//#define FALSE ((int8_t) 0)
//#define TRUE ((int8_t) 1)
//#endif
//#define ERROR ((int8_t) -1)
//#define SUCCESS ((int8_t) 1)

/* IO PINS */

// See Speaker.h
#define SPEAKER_PIN 8

// See Clock.h
#define TM1637_CLK 2
#define TM1637_DIO 3

// See TimerFSM.h
#define STRIKE1_PIN 12
#define STRIKE2_PIN 13

// See Consequence.h
#define CONSEQUENCE_PIN 4

/**
 * @brief Configures all required board peripherals and GPIO.
 */
void BOARD_Init();

#endif	/* BOARD_H */

