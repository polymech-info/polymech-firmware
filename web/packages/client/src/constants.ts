/**
 * Interval in milliseconds for polling REST API endpoints 
 * (Coils, Registers, SystemInfo).
 */
export const REST_POLLING_INTERVAL_MS = 2000;

/**
 * WebSocket reconnect attempt interval in milliseconds.
 */
export const WS_RECONNECT_INTERVAL_MS = 5000;

/**
 * WebSocket register refresh interval in milliseconds.
 */
export const WS_REGISTER_REFRESH_INTERVAL_MS = 500;

// Add new constant for polling
export const WS_REGISTER_POLL_INTERVAL_MS = 2500; // Interval for polling all registers via WebSocket

// Add other constants here as needed 

export const WS_HEARTBEAT_INTERVAL_MS = 10000;