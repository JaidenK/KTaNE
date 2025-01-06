#include <Arduino.h>
#include "KTaNE.h"
#include "BOARD_Timer.h"
#include "Speaker.h"
#include "Clock.h"
#include "HardwareTestHarness.h"

void Module_PerformSelfTest()
{
    // TODO 7seg Display Test
    //display.setBrightness(5);
    //display.clear();
    digitalWrite(STRIKE1_PIN,LOW);  
    digitalWrite(STRIKE2_PIN,LOW); 
    digitalWrite(CONSEQUENCE_PIN,0);

    for(uint8_t pos = 0; pos < 4; pos++)
    {
        for(uint8_t i = 0; i < 10; i++)
        {
            //display.showNumberDec(i, 0, 1, pos);
            //delay(50);
        }
    }

    Clock_ShowTime_ms(754000); // 1234
    
    digitalWrite(STRIKE1_PIN,HIGH);   
    digitalWrite(CONSEQUENCE_PIN,HIGH);    
    delay(500);
    digitalWrite(STRIKE1_PIN,LOW);  

    digitalWrite(STRIKE2_PIN,HIGH);     
    delay(500);
    digitalWrite(STRIKE2_PIN,LOW);  
    digitalWrite(CONSEQUENCE_PIN,LOW);  

    Speaker_BeepBlocking();
       

    Serial.println(F("Self test performed."));

    TEST_RESULTS = SELFTEST_SUCCESS;
    STATUS |= _BV(STS_RESULT_READY);
}
