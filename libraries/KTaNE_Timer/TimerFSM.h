#ifndef _TIMERFSM_H_
#define _TIMERFSM_H_

uint8_t InitTimerFSM(uint8_t Priority);
uint8_t PostTimerFSM(ES_Event ThisEvent);
ES_Event RunTimerFSM(ES_Event ThisEvent);

#endif /* _TIMERFSM_H_ */