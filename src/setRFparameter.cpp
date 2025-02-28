#include "setRFparameter.h"

//uint8_t setRFpara::defaultValues[NUM_GROUPS][RF_NUM_DEFAULT] = {{0}};
const ProtocolParams *txParams;
//const ProtocolParams *rxParams;


void setRFpara::setRFParameters(uint8_t workmode)
{
    switch ((RFworkMode)workmode)
    {
    //case RFworkMode::HANS_RECEIVER:
    case RFworkMode::HANS_BOTH:
    case RFworkMode::HANS_TRANSMITTER:
    case RFworkMode::HOPO_HANS:
       // rxParams = &PROTOCOL_HANS;
        txParams = &PROTOCOL_HANS;
        mySerial.println(F(" TX IS HANS."));
        break;
    //case RFworkMode::HOPO_RECEIVER:
    case RFworkMode::HOPO_TRANSMITTER:
    case RFworkMode::HANS_HOPO:
        //rxParams = &PROTOCOL_HOPO;
        txParams = &PROTOCOL_HOPO;
        mySerial.println(F(" TX IS HOPO."));
        break;   
    
    default:
        //rxParams = &PROTOCOL_HANS;
        txParams = &PROTOCOL_HANS;
        mySerial.println(F("DEFAULT, TX IS HANS."));
        break;
    }
}

