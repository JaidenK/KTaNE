#ifndef _KEYPADSFSM_H_
#define _KEYPADSFSM_H_

uint8_t InitKeypadsFSM(uint8_t Priority);
uint8_t PostKeypadsFSM(ES_Event ThisEvent);
ES_Event RunKeypadsFSM(ES_Event ThisEvent);

#endif /* _KEYPADSFSM_H_ */