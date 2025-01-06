#ifndef _CONSEQUENCES_H_
#define _CONSEQUENCES_H_

/**
 * @brief Arms the consequence hardware.
 * 
 * Currently this does nothing because we just have
 * the normally-open relay that we'll pulse to indicate
 * game over. If we ever add a heartbeat signal generation,
 * then this would start that. 
 */
void Consequences_Arm(void);

/**
 * @brief Ensures the consequence hardware is in a safe state.
 * 
 * Currently just turns off the relay. If we were using
 * a heartbeat signal I have no idea what we'd do here.
 */
void Consequences_Safe(void);

/**
 * @brief Causes the consequence hardware to perform its behavior. 
 * 
 * Currently just turns on the relay. If we were generating a 
 * heartbeat, we could simply stop generating it. Perhaps the 
 * heartbeat can be serial? Then we could implement a safe command.
 * That would also make it easy to hook up to a normal Arduino
 * for people who wanted to make custom consequence hardware. 
 */
void Consequences_Fire(void);

#endif /* _CONSEQUENCES_H_ */