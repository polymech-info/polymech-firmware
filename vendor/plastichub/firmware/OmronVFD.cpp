#include "OmronVFD.h"
#include "ModbusBridge.h"
#include "./components/OmronMX2.h"
#include "app.h"

#define valA001 3 // A001 Frequency reference source = 03 (no need to change)
#define valA002 3 // A002 Source of the “Move” command = 03 (no need to change)
#define valC026 5 // C026 Relay output function 5 (AL: error signal) = 05
#define DEF_FC_MAX_FREQ 500

short OmronVFD::onStart()
{

#ifdef COOLING_RELAY
    digitalWrite(COOLING_RELAY, HIGH);
#endif
#ifdef COOLING_RELAY2
    digitalWrite(COOLING_RELAY2, HIGH);
#endif
#ifdef FEEDSCREW_RELAY
    digitalWrite(FEEDSCREW_RELAY, HIGH);
#endif
    return E_OK;
}

short OmronVFD::onStop()
{
#ifdef COOLING_RELAY
    digitalWrite(COOLING_RELAY, 0);
#endif
#ifdef COOLING_RELAY2
    digitalWrite(COOLING_RELAY2, 0);
#endif
#ifdef FEEDSCREW_RELAY
    digitalWrite(FEEDSCREW_RELAY, 0);
#endif
    return E_OK;
}

void OmronVFD::doTest()
{
    Serial.println(" Do VFD Tests ");
    pollState = true;
    // forward();
    //  ping();
    setTargetFreq(50);
    run();

    // reverse();
    // run();

    /*
    owner->timer.in(
        10000, [](OmronVFD *me) -> void {
            me->stop();
        },
        this);
        */

    // stop();
    // configure();
}
uint16_t OmronVFD::configure()
{
    // write_Single(MX2_A001, valA001);
    // write_Single(MX2_A002, valA002);
    // write_Single(MX2_C026, valC026); // C026 Relay output function 5 (AL: error signal) = 05
    //  write_Single(MX2_A004, DEF_FC_MAX_FREQ / 10); // A004 setting the maximum frequency
    //  progReg32(MX2_F002, (char *)" F002 ", FC_ACCEL_TIME);        // F002 Acceleration Time
    //  progReg32(MX2_F002, (char *)" F003 ", FC_DEACCEL_TIME);      // F003 Acceleration Braking

    for (int i = 0; 0 < MB_N_R; i++)
    {
        modbus->mb->R[i] = 0;
    }

    for (int i = 0; 0 < MB_N_C; i++)
    {
        modbus->mb->C[i] = false;
    }
}
uint16_t OmronVFD::updateState()
{
    // readSingle_16(MX2_STATE);
    // readSingle_16(MX2_STATUS);
    if (now - readStateTS > OMRON_MX2_STATE_INTERVAL)
    {
        read_16(1, 5, MB_QUERY_TYPE_STATUS_POLL);
        readStateTS = now;
        // readSingle_16(0x1003);
    }

    // readSingle_16(MX2_CURRENT_FR);
    // readSingle_16(MX2_AMPERAGE);
}

////////////////////////////////////////////////////////////////////////////
//
// HMI only (Manual = A2 = 2)
uint16_t OmronVFD::stop()
{
    onStop();
    return write_Bit(MX2_START, 0);
}

uint16_t OmronVFD::run()
{
    onStart();
    return write_Bit(MX2_START, 1);
}

uint16_t OmronVFD::reverse()
{
    return write_Bit(MX2_SET_DIR, 1);
}
uint16_t OmronVFD::forward()
{
    return write_Bit(MX2_SET_DIR, 0);
}

uint16_t OmronVFD::setTargetFreq(uint16_t freq)
{
    return write_Single(MX2_TARGET_FR, freq * 100);
}

////////////////////////////////////////////////////////////////////////////
//
// Addon impl.
short OmronVFD::setup()
{
    // configure();
    onStop();
}

short OmronVFD::loop()
{
    modbusLoop();
    status.loop();
}

short OmronVFD::debug(Stream *stream)
{
    //*stream << this->name << ":" << this->ok();
    return false;
}
short OmronVFD::info(Stream *stream)
{
    //*stream << this->name << "\n\t : " SPACE("Pin:" << MOTOR_IDLE_PIN);
    return false;
}

void OmronVFD::init()
{
}
