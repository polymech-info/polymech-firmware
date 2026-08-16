#ifndef IPLUNGER_EVENTS_H
#define IPLUNGER_EVENTS_H

class Plunger;

enum class PlungerEvent : uint8_t {
    STATE_CHANGED = 0,
    JAM_DETECTED = 1,
    JAM_CLEARED = 2,
    OPERATION_STARTED = 3,
    OPERATION_COMPLETED = 4,
    HOMING_STARTED = 5,
    HOMING_COMPLETED = 6,
    PLUNGING_STARTED = 7,
    PLUNGING_COMPLETED = 8,
    FILL_STARTED = 9,
    FILL_COMPLETED = 10,
    RECORD_STARTED = 11,
    RECORD_COMPLETED = 12,
    REPLAY_STARTED = 13,
    REPLAY_COMPLETED = 14,
    POST_FLOW_STARTED = 15,
    POST_FLOW_COMPLETED = 16,
    AUTO_MODE_ENABLED = 17,
    AUTO_MODE_DISABLED = 18
};

/**
 * @brief An interface for a delegate to receive events from Plunger.
 *
 * This allows the plunger object to notify an owner or manager of state changes
 * and operational events without requiring a direct dependency on the concrete
 * class of that manager (e.g., PHApp).
 */
class IPlungerEvents {
public:
    virtual ~IPlungerEvents() = default;

    /**
     * @brief Called when a plunger event occurs.
     * 
     * @param instance Pointer to the Plunger instance that triggered the event
     * @param event The type of event that occurred
     * @param val Optional value associated with the event (default: 0)
     */
    virtual void onPlungerEvent(Plunger* instance, PlungerEvent event, int16_t val = 0) {}
};

#endif // IPLUNGER_EVENTS_H
