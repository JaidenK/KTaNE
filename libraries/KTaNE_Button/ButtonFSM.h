#ifndef _BUTTONFSM_H_
#define _BUTTONFSM_H_

uint8_t InitButtonFSM(uint8_t Priority);
uint8_t PostButtonFSM(ES_Event ThisEvent);
ES_Event RunButtonFSM(ES_Event ThisEvent);

#endif /* _BUTTONFSM_H_ */