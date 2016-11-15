// OMMoCoPrint.h

#ifndef _OMMOCOPRINT_h
#define _OMMOCOPRINT_h

#if defined(ARDUINO) && ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif
#include "OMMoCoNode.h"

class OMMoCoPrintClass : public Print
{
 private:
     OMMoCoNode *m_node;

 protected:


 public:
    OMMoCoPrintClass(OMMoCoNode *node);
    void init();
    virtual size_t write(uint8_t);
};

#endif

