#include "ofxTWELite.h"

ofxTWELite::ofxTWELite() {}
ofxTWELite::~ofxTWELite() {
    waitForThread();
    port.close();
}

bool ofxTWELite::connect(const string & device, unsigned int baud) {
    port.enumerateDevices();
    if (port.setup(device.c_str(), baud)) {
        startThread();
        return true;
    }
    return false;
}

void ofxTWELite::disconnect() {
    port.close();
}

void ofxTWELite::analogWrite(uint8_t device_id, uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4) {
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
    << TWE_SKIP_CHECK << TWE_CR << TWE_LF;
    port.writeBytes((unsigned char *)ss.str().c_str(), TWE_WRITE_COMMAND_LENGTH);
}

void ofxTWELite::analogWrite(uint8_t device_id, uint8_t pin, uint16_t value) {
    int v[TWE_TOTAL_PWM] = {TWE_ANALOG_IGNORE, TWE_ANALOG_IGNORE, TWE_ANALOG_IGNORE, TWE_ANALOG_IGNORE};
    v[pin] = value;
    analogWrite(device_id, v[0], v[1], v[2], v[3]);
}

void ofxTWELite::analogWrite(uint16_t v1, uint16_t v2, uint16_t v3, uint16_t v4) {
    analogWrite(TWE_ALL_DEVICE_ID, v1, v2, v3, v4);
}

void ofxTWELite::analogWrite(uint8_t pin, uint16_t value) {
    analogWrite(TWE_ALL_DEVICE_ID, pin, value);
}

void ofxTWELite::digitalWrite(uint8_t device_id, PINLEVEL v1, PINLEVEL v2, PINLEVEL v3, PINLEVEL v4, PINLEVEL p1, PINLEVEL p2, PINLEVEL p3, PINLEVEL p4) {
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
    << TWE_SKIP_CHECK << TWE_CR << TWE_LF;
    port.writeBytes((unsigned char *)ss.str().c_str(), TWE_WRITE_COMMAND_LENGTH);
}

void ofxTWELite::digitalWrite(uint8_t device_id, uint8_t pin, PINLEVEL value) {
    bool p[TWE_TOTAL_DIGITAL_OUT] = { LOW };
    p[pin] = HIGH;
    bool v[TWE_TOTAL_DIGITAL_OUT] = { LOW };
    v[pin] = value;
    digitalWrite(device_id, v[0], v[1], v[2], v[3], p[0], p[1], p[2], p[3]);
}

void ofxTWELite::digitalWrite(PINLEVEL v1, PINLEVEL v2, PINLEVEL v3, PINLEVEL v4) {
    digitalWrite(TWE_ALL_DEVICE_ID, v1, v2, v3, v4);
}

void ofxTWELite::digitalWrite(uint8_t pin, PINLEVEL value) {
    digitalWrite(TWE_ALL_DEVICE_ID, pin, value);
}

bool ofxTWELite::analogRead(int * value, uint8_t device_id, uint8_t pin) {
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

bool ofxTWELite::digitalRead(PINLEVEL * value, uint8_t device_id, uint8_t pin) {
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

bool ofxTWELite::getState(ofxTWELiteState * state, uint8_t device_id) {
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

void ofxTWELite::threadedFunction() {
    if (!port.isInitialized()) return;
    static string bytes;
    while(isThreadRunning()) {
        if (lock()) {
            if (port.available()) {
                unsigned char byte = port.readByte();
                if (byte == TWE_HEAD_MARKER || bytes.length() > TWE_READ_COMMAND_LENGTH) bytes.clear();
                else if (byte == TWE_CR) processBytes(bytes.c_str());
                else bytes += byte;
            }
            unlock();
        }
    }
}

void ofxTWELite::processBytes(const char * buf) {
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
            for (int i = 0; i < TWE_TOTAL_DIGITAL_IN; i++)
                state->digital_input[i] = (di >> i) & 1;
            
            uint8_t dp = bytes[16];
            for (int i = 0; i < TWE_TOTAL_DIGITAL_IN; i++)
                state->digital_change[i] = (dp >> i) & 1;
            
            uint8_t af = bytes[21];
            for (int i = 0; i < TWE_TOTAL_ANALOG_IN; i++)
                state->analog_input[i] = (bytes[17 + i] *  4 + af) * 4;
            
            ofNotifyEvent(stateReceived, *state);
        }
    }
}