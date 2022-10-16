#ifndef _SKETCHPADFSM_H_
#define _SKETCHPADFSM_H_

uint8_t InitSketchpadFSM(uint8_t Priority);
uint8_t PostSketchpadFSM(ES_Event ThisEvent);
ES_Event RunSketchpadFSM(ES_Event ThisEvent);

#endif /* _SKETCHPADFSM_H_ */