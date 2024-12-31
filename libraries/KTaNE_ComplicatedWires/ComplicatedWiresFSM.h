#ifndef _KEYPADSFSM_H_
#define _KEYPADSFSM_H_

uint8_t InitComplicatedWiresFSM(uint8_t Priority);
uint8_t PostComplicatedWiresFSM(ES_Event ThisEvent);
ES_Event RunComplicatedWiresFSM(ES_Event ThisEvent);

#endif /* _KEYPADSFSM_H_ */