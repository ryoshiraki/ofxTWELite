#pragma once

#include "ofMain.h"

#define tohex(w) std::hex<<std::setfill('0')<<std::setw(w)<<std::uppercase
#define tolevel(l) (l?"HIGH":"LOW")

#define TWE_HEAD_MARKER ':'
#define TWE_VERSION 0x01

#define TWE_TOTAL_DIGITAL_PINS 4
#define TWE_TOTAL_ANALOG_PINS 4

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

#define TWE_SKIP_CHECK "X\r\n"

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
    PINLEVEL digital_input[TWE_TOTAL_DIGITAL_PINS];
    PINLEVEL digital_change[TWE_TOTAL_ANALOG_PINS];
    int analog_input[TWE_TOTAL_ANALOG_PINS];
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
    ofxTWELite() {}
    virtual ~ofxTWELite() {
        waitForThread();
        port.close();
    }
    
    bool connect(const string & device, unsigned int baud) {
        port.enumerateDevices();
        if (port.setup(device.c_str(), baud)) {
            startThread();
            return true;
        }
        return false;
    }
    
    void disconnect() {
        port.close();
    }

    void analogWrite(uint8_t device_id, uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4) {
        if (!port.isInitialized()) return;
        stringstream ss;
        ss << TWE_HEAD_MARKER
        << tohex(2) << static_cast<int>(device_id)
        << tohex(2) << TWE_WRITE
        << tohex(2) << TWE_VERSION
        << tohex(2) << TWE_DIGITAL_IGNORE
        << tohex(2) << TWE_DIGITAL_IGNORE
        << tohex(4) << static_cast<int>(v1)
        << tohex(4) << static_cast<int>(v2)
        << tohex(4) << static_cast<int>(v3)
        << tohex(4) << static_cast<int>(v4)
        << TWE_SKIP_CHECK;
        cout << ss.str() <<  ":" << ss.str().length() << endl;
        port.writeBytes((unsigned char *)ss.str().c_str(), TWE_WRITE_COMMAND_LENGTH);
    }
    
    void analogWrite(uint8_t device_id, uint8_t pin, uint16_t value) {
        int v[TWE_TOTAL_ANALOG_PINS] = {TWE_ANALOG_IGNORE, TWE_ANALOG_IGNORE, TWE_ANALOG_IGNORE, TWE_ANALOG_IGNORE};
        v[pin] = value;
        analogWrite(device_id, v[0], v[1], v[2], v[3]);
    }
    
    void analogWrite(uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4) {
        analogWrite(TWE_ALL_DEVICE_ID, v1, v2, v3, v4);
    }
    
    void analogWrite(uint8_t pin, uint16_t value) {
        analogWrite(TWE_ALL_DEVICE_ID, pin, value);
    }
    
    void digitalWrite(uint8_t device_id, PINLEVEL v1, PINLEVEL v2, PINLEVEL v3, PINLEVEL v4, PINLEVEL p1 = HIGH, PINLEVEL p2 = HIGH, PINLEVEL p3 = HIGH, PINLEVEL p4 = HIGH) {
        if (!port.isInitialized()) return;
        stringstream ss;
        ss << TWE_HEAD_MARKER
        << tohex(2) << static_cast<int>(device_id)
        << tohex(2) << TWE_WRITE
        << tohex(2) << TWE_VERSION
        << tohex(2) << ((v1 << 0) | (v2 << 1) | (v3 << 2) | (v4 << 3))
        << tohex(2) << ((p1 << 0) | (p2 << 1) | (p3 << 2) | (p4 << 3))
        << tohex(4) << TWE_ANALOG_IGNORE
        << tohex(4) << TWE_ANALOG_IGNORE
        << tohex(4) << TWE_ANALOG_IGNORE
        << tohex(4) << TWE_ANALOG_IGNORE
        << TWE_SKIP_CHECK;
        cout << ss.str() <<  ":" << ss.str().length() << endl;
        port.writeBytes((unsigned char *)ss.str().c_str(), TWE_WRITE_COMMAND_LENGTH);
    }
    
    void digitalWrite(uint8_t device_id, uint8_t pin, PINLEVEL value) {
        bool p[TWE_TOTAL_DIGITAL_PINS] = { LOW };
        p[pin] = HIGH;
        bool v[TWE_TOTAL_DIGITAL_PINS] = { LOW };
        v[pin] = value;
        digitalWrite(device_id, v[0], v[1], v[2], v[3], p[0], p[1], p[2], p[3]);
    }
    
    void digitalWrite(PINLEVEL v1, PINLEVEL v2, PINLEVEL v3, PINLEVEL v4) {
        digitalWrite(TWE_ALL_DEVICE_ID, v1, v2, v3, v4);
    }
    
    void digitalWrite(uint8_t pin, PINLEVEL value) {
        digitalWrite(TWE_ALL_DEVICE_ID, pin, value);
    }
    
    bool analogRead(int * value, uint8_t device_id, uint8_t pin) {
        lock();
        if (states.find(device_id) != states.end()) {
            *value = states[device_id].analog_input[pin];
            unlock();
            return true;
        } else {
            unlock();
            return false;
        }
    }
   
    bool digitalRead(PINLEVEL * value, uint8_t device_id, uint8_t pin) {
        lock();
        if (states.find(device_id) != states.end()) {
            *value = states[device_id].digital_input[pin];
            unlock();
            return true;
        } else {
            unlock();
            return false;
        }
    }
    
    bool getState(ofxTWELiteState * state, uint8_t device_id) {
        lock();
        if (states.find(device_id) != states.end()) {
            *state = states[device_id];
            unlock();
            return true;
        } else {
            unlock();
            return false;
        }
    }

    ofEvent<ofxTWELiteState> stateReceived;
    
protected:
    ofSerial port;
    map<int, ofxTWELiteState> states;

private:
    void threadedFunction() {
        if (!port.isInitialized()) return;
        static string bytes;
        while(isThreadRunning()) {
            if (lock()) {
                if (port.available()) {
                    unsigned char byte = port.readByte();
                    if (byte == ':') bytes.clear();
                    else if (byte == '\r') processBytes(bytes.c_str());
                    else bytes += byte;
                }
                unlock();
            }
        }
    }
    
    void processBytes(const char * buf) {
        vector<uint8_t> bytes;
        int len = strlen(buf);
        for (int i = 0; i < len; i += 2) {
            unsigned int x;
            sscanf((char *)(buf + i), "%02x", &x);
            bytes.push_back(x);
        }
        
        uint8_t sum = accumulate(bytes.begin(), bytes.end(), 0);
        if (sum != 0x00)
            cerr << "checksum error : " << buf << endl;
        else {
            if (bytes[1] == TWE_READ) {
                ofxTWELiteState * state;
                if (states.find(bytes[0]) == states.end()) {
                    states.insert(make_pair(bytes[0], ofxTWELiteState()));
                    state = &states[bytes[0]];
                } else
                    state = &states[bytes[0]];
                
                state->device_id = bytes[0];
                state->cmd = bytes[1];
                state->packet_identifier = bytes[2];
                state->protocol_version = bytes[3];
                state->LQI = (bytes[4] * 7 - 1970) / 20.0f;
                state->identification_id = (((((bytes[5] << 8) | bytes[6]) << 8) | bytes[7]) << 8) | bytes[8];
                state->time_stamp = (bytes[10] << 8) | bytes[11];
                state->relay_flag = bytes[12];
                state->power_voltage = (bytes[13] << 8) | bytes[14];
                
                uint8_t di = bytes[15];
                for (int i = 0; i < TWE_TOTAL_DIGITAL_PINS; i++)
                    state->digital_input[i] = (di >> i) & 1;
                
                uint8_t dp = bytes[16];
                for (int i = 0; i < TWE_TOTAL_DIGITAL_PINS; i++)
                    state->digital_change[i] = (dp >> i) & 1;
                
                uint8_t af = bytes[21];
                for (int i = 0; i < TWE_TOTAL_ANALOG_PINS; i++)
                    state->analog_input[i] = (bytes[17 + i] *  4 + af) * 4;
                
                ofNotifyEvent(stateReceived, *state);
            }
        }
    }
};