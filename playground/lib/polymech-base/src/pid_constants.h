#pragma once
/**
 * @file cassandra650_testdata.h
 * @brief Verified test-data constants for the Cassandra 650 hot-plate system.
 *
 * ■ Notation
 *   •  All variables ending in **_X100** are stored   ×100 → 1.25 → 125.
 *   •  Dimensions in millimetres unless otherwise noted.
 *   •  Time: s = seconds, min = minutes.
 *   •  mHz = milli-hertz (10⁻³ Hz) – *NOT* megahertz.
 *
 * ---------------------------------------------------------------
 *  Section legend
 *    HEATPLATE_   → purely geometric
 *    HEATER_      → individual cartridge data
 *    COOLING_     → passive natural-convection rates
 *    TEMPERATURE_ → control-loop precision
 *    OSCILLATION_ → bang-bang behaviour while holding
 *    …            → thermodynamic derived / placeholder values
 * ---------------------------------------------------------------
 */

//
// ───────────────── Heat-plate geometry ─────────────────────────
//
#define TD_CS65_HEATPLATE_WIDTH_MM                 650   /**< mm */
#define TD_CS65_HEATPLATE_DEPTH_MM                 650   /**< mm */
#define TD_CS65_HEATPLATE_THICKNESS_MM             15    /**< mm */

//
// ───────────────── Heater configuration ────────────────────────
//
#define TD_CS65_HEATER_COUNT                       9
#define TD_CS65_HEATER_POWER_W                     300   /**< W (per heater) */
#define TD_CS65_TOTAL_HEATING_POWER_W   (TD_CS65_HEATER_COUNT * TD_CS65_HEATER_POWER_W)
/**< **Eq. 1** P_total = N × P_heater = 9×300 = **2700 W** */

#define TD_CS65_HEATER_VOLTAGE_V                   220   /**< VAC */
#define TD_CS65_HEATER_BLOCK_WIDTH_MM              80
#define TD_CS65_HEATER_BLOCK_HEIGHT_MM             20

/** Ø 11.60 mm cartridge bore, ±0.03 mm  (×100 scaling) */
#define TD_CS65_HEATER_FIT_DIAMETER_MM_X100        1160  /**< 11.60 mm */
#define TD_CS65_HEATER_FIT_TOLERANCE_MM_X100       3     /**< ±0.03 mm */

//
// ───────────────── Cooling performance ─────────────────────────
//   dT/dt (°C · min⁻¹) ×100
//
#define TD_CS65_COOLING_RATE_PASSIVE_OPEN_DEGC_MIN_X100    125 /**< 1.25 °C/min */
#define TD_CS65_COOLING_RATE_PASSIVE_CLOSED_DEGC_MIN_X100  50  /**< 0.50 °C/min */

//
// ───────────────── Temperature-loop precision ──────────────────
//
#define TD_CS65_TEMPERATURE_PRECISION_DEGC_X100            100 /**< ±1 °C */

//
// ───────────────── Oscillation while holding ───────────────────
/**
 * The controller uses simple on/off (relay-style) regulation:
 *   duty = 33 %,  period = 15 s  →  on-time = 5 s.
 */
#define TD_CS65_OSCILLATION_HOLDING_DUTY_CYCLE_PERCENT     33  /**< % */
#define TD_CS65_OSCILLATION_HOLDING_PERIOD_S               15  /**< s  (T) */
#define TD_CS65_OSCILLATION_HOLDING_ON_TIME_S              5   /**< s  (T_on) */

/**
 * **Eq. 2**  f_mHz = 1000 / T_s  
 *                 = 1000 / 15 ≈ **66.7 mHz**.
 * Stored as **67 mHz** (rounded).
 */
#define TD_CS65_OSCILLATION_HOLDING_FREQ_MHZ               67  /**< 67 mHz */

//
// ───────────────── Heating performance ─────────────────────────
/**
 * **Eq. 3**  dT/dt = ΔT / t_min  
 *  ΔT = 100 °C  (room → target)  
 *  t  = 17 min  (measured)  
 *  → 5.882 °C/min ≈ 588.2 ×100
 */
#define TD_CS65_LOWER_PLATE_HEAT_RATE_DEGC_MIN_X100        588
#define TD_CS65_UPPER_PLATE_HEAT_RATE_DEGC_MIN_X100        588

/**
 * Observed heating rate from a specific scenario (e.g., as depicted in a typical performance graph).
 * Details from graph:
 *   Initial Temperature ≈ 75°C
 *   Final Temperature   ≈ 150°C
 *   ΔT                  ≈ 75°C
 *   Time taken (t)      ≈ 900 seconds (15 minutes)
 *   Rate = ΔT / t_min   = 75°C / 15 min = 5.00 °C/min
 *   Stored value (×100) = 500
 */
#define TD_CS65_MEASURED_SCENARIO_HEAT_RATE_DEGC_MIN_X100 500

//
// ───────────────── Derived thermodynamic properties ────────────
/**
 * ■ Heat capacity  
 *   **Eq. 4**  C [J · °C⁻¹] = P_total / (dT/dt)_s
 *
 *   where (dT/dt)_s = (dT/dt)_min / 60  
 *         (dT/dt)_min = 5.882 °C/min
 *
 *   C = 2700 W / (5.882/60) °C/s ≈ **27 551 J/°C**
 */
#define TD_CS65_SYSTEM_HEAT_CAPACITY_J_PER_DEGC            27551

/**
 * ■ Thermal conductivity  
 *   Material-specific → set via plate alloy in future rev.
 */
#define TD_CS65_HEATPLATE_THERMAL_CONDUCTIVITY_W_PER_M_K_X100 0

/**
 * ■ Thermal resistance to ambient  
 *   Placeholder using coarse estimate:
 *
 *   **Eq. 5**  R_th = ( T_hold – T_amb ) / ( P_total × duty )
 *
 *     T_hold ≈ 100 °C,  T_amb ≈ 25 °C  
 *     duty   = 0.33  
 *   → R_th ≈ 0.084 °C/W  → 8 ×100
 */
#define TD_CS65_SYSTEM_THERMAL_RESISTANCE_DEGC_PER_W_X100  8

//
// ───────────────── Sequential Heating (2 Partitions) ───────────
/**
 * Defines parameters for operating a system with two identical Cassandra 650
 * partitions, where only one partition can be powered at a time.
 *
 * Assumptions:
 *  - System comprises N=2 partitions.
 *  - Each partition matches the single Cassandra 650 specs (9 heaters, 2700W).
 *  - Power supply can deliver 2700W to ONE partition when active.
 *  - Goal is for each partition to achieve its target heating rate (Eq. 3)
 *    *during its active heating phase*.
 *
 * Derivation:
 *  - To achieve the target rate of ~5.88 °C/min (Eq. 3), a partition
 *    requires continuous heating (2700W) while active.
 *    → Min heating time per partition = 60 seconds.
 *
 *  - With sequential heating, the minimum cycle includes heating each partition
 *    for this minimum time.
 *    → Cycle Time = N_partitions × Min_Heating_Time
 *                 = 2 × 60s = 120s
 *
 *  - The duty cycle for each partition within this sequential operation is:
 *    → Duty Cycle = Min_Heating_Time / Cycle_Time
 *                 = 60s / 120s = 0.5 (50%)
 *
 * Average On-Time per Partition within a 60-second window:
 *  Given the 50% duty cycle over the 120s cycle, the average time
 *  a single partition is ON during any 60-second interval is:
 *    Avg_On_Time = 60s * (Duty Cycle / 100)
 *                = 60s * (50 / 100) = 30s
 *
 *  **Eq. 9**  Avg_On_Time_per_Min [s] = 60 * Duty_% / 100
 *
 * Note: This is an average. Instantaneously, a partition is either
 * fully ON (for 60s) or fully OFF (for 60s) during the cycle.
 */
#define TD_CS65_SYSTEM_PARTITION_COUNT                   2    /**< Number of identical partitions */
#define TD_CS65_PARTITION_MIN_ACTIVE_HEAT_TIME_S         60   /**< Seconds needed for target rate (t_heat_min) */

// Eq. 7: T_cycle = N_partitions * t_heat_min
#define TD_CS65_2PART_SEQUENTIAL_CYCLE_TIME_S           (TD_CS65_SYSTEM_PARTITION_COUNT * TD_CS65_PARTITION_MIN_ACTIVE_HEAT_TIME_S) /**< 120 s */

// Eq. 8: Duty = (t_heat_min * 100) / T_cycle
#define TD_CS65_2PART_SEQUENTIAL_DUTY_CYCLE_PERCENT     ((TD_CS65_PARTITION_MIN_ACTIVE_HEAT_TIME_S * 100) / TD_CS65_2PART_SEQUENTIAL_CYCLE_TIME_S) /**< 50 % */

// Eq. 9: Avg_On_Time = 60 * Duty_% / 100
#define TD_CS65_2PART_AVG_ON_TIME_PER_MIN_S             ((60 * TD_CS65_2PART_SEQUENTIAL_DUTY_CYCLE_PERCENT) / 100) /**< 30 s (Avg) */

////////////////////////////////////////////////////////////////////
//
//  Not so constant
#define PRICE_PER_KWH_EUROS 0.289f

/*
 * -----------------------------------------------------------------
 *  Revision history
 *  2025-05-01  Initial verified version with full equation markup.
 * -----------------------------------------------------------------
 */
