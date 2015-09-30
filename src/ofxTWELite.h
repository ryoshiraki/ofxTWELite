#pragma once

#include "ofMain.h"

#define tohex(w) std::hex<<std::setfill('0')<<std::setw(w)<<std::uppercase
#define tolevel(l) (l?"HIGH":"LOW")

#define TWE_HEAD_MARKER ':'
#define TWE_VERSION 0x01

#define TWE_TOTAL_DIGITAL_IN 4
#define TWE_TOTAL_DIGITAL_OUT 4
#define TWE_TOTAL_ANALOG_IN 4
#define TWE_TOTAL_PWM 4

#define TWE_WRITE_COMMAND_LENGTH 31
#define TWE_READ_COMMAND_LENGTH 51

#define TWE_WRITE 0x80
#define TWE_READ 0x81
#define TWE_ANALOG_IGNORE 0xffff
#define TWE_DIGITAL_IGNORE 0x00

#define TWE_ANALOG_WRITE_MAX 0x0400

#define TWE_ALL_DEVICE_ID 0x78
#define TWE_MAX_DEVICE_ID 0x64
#define TWE_REPEATER_DEVICE_ID 0x7A

#define TWE_SKIP_CHECK 'X'
#define TWE_CR '\r'
#define TWE_LF '\n'

typedef bool PINLEVEL;
enum { HIGH = 1, LOW = 0 };

struct ofxTWELiteState {
    uint8_t device_id;
    uint8_t cmd;
    uint8_t packet_identifier;
    uint8_t protocol_version;
    float LQI;
    uint64_t identification_id;
    uint8_t destination_id;
    uint16_t time_stamp;
    bool relay_flag;
    int power_voltage;
    PINLEVEL digital_input[TWE_TOTAL_DIGITAL_IN];
    PINLEVEL digital_change[TWE_TOTAL_DIGITAL_IN];
    int analog_input[TWE_TOTAL_ANALOG_IN];
};

static inline std::ostream &operator<<(std::ostream & os, const ofxTWELiteState & dt) {
    os
    << "device id        : " << tohex(2) << static_cast<int>(dt.device_id) << endl
    << "command          : " << tohex(2) << static_cast<int>(dt.cmd) << endl
    << "packet idetifier : " << tohex(2) << static_cast<int>(dt.packet_identifier) << endl
    << "protocol version : " << tohex(2) << static_cast<int>(dt.protocol_version) << endl
    << "LQI              : " << dt.LQI << " (dBm)" << endl
    << "idetifier id     : " << tohex(8) << static_cast<int>(dt.identification_id) << endl
    << "destination id   : " << tohex(2) << static_cast<int>(dt.destination_id) << endl
    << "time stamp       : " << tohex(4) << static_cast<int>(dt.time_stamp) << endl
    << "relay flag       : " << tohex(2) << static_cast<int>(dt.relay_flag) << endl
    << "power voltage    : " << std::dec << dt.power_voltage << " (mV)" << endl
    << "digital input    : " << tolevel(dt.digital_input[0]) << ", " << tolevel(dt.digital_input[1]) << ", " << tolevel(dt.digital_input[2]) << ", " << tolevel(dt.digital_input[3]) << endl
    << "digital flag     : " << tolevel(dt.digital_change[0]) << ", " << tolevel(dt.digital_change[1]) << ", " << tolevel(dt.digital_change[2]) << ", " << tolevel(dt.digital_change[3]) << endl
    << "analog input     : " << dt.analog_input[0] << ", " << dt.analog_input[1] << ", " << dt.analog_input[2] << ", " << dt.analog_input[3] << " (mV)" << endl;
    return os;
}

class ofxTWELite : ofThread {
public:
    ofxTWELite();
    virtual ~ofxTWELite();
    
    bool connect(const string & device, unsigned int baud);
    void disconnect();

    void analogWrite(uint8_t device_id, uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4);
    void analogWrite(uint8_t device_id, uint8_t pin, uint16_t value);
    void analogWrite(uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4);
    void analogWrite(uint8_t pin, uint16_t value);
    
    void digitalWrite(uint8_t device_id, PINLEVEL v1, PINLEVEL v2, PINLEVEL v3, PINLEVEL v4, PINLEVEL p1 = HIGH, PINLEVEL p2 = HIGH, PINLEVEL p3 = HIGH, PINLEVEL p4 = HIGH);
    void digitalWrite(uint8_t device_id, uint8_t pin, PINLEVEL value);
    void digitalWrite(PINLEVEL v1, PINLEVEL v2, PINLEVEL v3, PINLEVEL v4);
    void digitalWrite(uint8_t pin, PINLEVEL value);
    
    bool analogRead(int * value, uint8_t device_id, uint8_t pin);
    bool digitalRead(PINLEVEL * value, uint8_t device_id, uint8_t pin);
    
    bool getState(ofxTWELiteState * state, uint8_t device_id);
    
    ofEvent<ofxTWELiteState> stateReceived;
    
protected:
    ofSerial port;
    map<int, ofxTWELiteState> states;

private:
    void threadedFunction();
    void processBytes(const char * buf);
};