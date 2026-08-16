// This file acts as the single source of truth for compile-time feature flags.
// By reading the VITE_... environment variables, it allows Vite to statically
// analyze the code and remove disabled features from the final bundle (tree-shaking).

export const compileTimeFlags = {
  ENABLE_OMRON_E5: import.meta.env.VITE_ENABLE_OMRON_E5 !== 'false',
  ENABLE_AMPERAGE_BUDGET_MANAGER: import.meta.env.VITE_ENABLE_AMPERAGE_BUDGET_MANAGER !== 'false',
  ENABLE_PROFILE_TEMPERATURE: import.meta.env.VITE_ENABLE_PROFILE_TEMPERATURE !== 'false',
  ENABLE_PROFILE_SIGNAL_PLOT: import.meta.env.VITE_ENABLE_PROFILE_SIGNAL_PLOT !== 'false',
  ENABLE_PRESS_CYLINDER: import.meta.env.VITE_ENABLE_PRESS_CYLINDER !== 'false',
  ENABLE_PROFILE_PRESSURE: import.meta.env.VITE_ENABLE_PROFILE_PRESSURE !== 'false',
  ENABLE_SAKO_VFD: import.meta.env.VITE_ENABLE_SAKO_VFD !== 'false',
};

// Derived feature checks
export const isProfilesEnabled = (flags: Record<string, boolean>): boolean => {
  return Boolean(
    flags.ENABLE_PROFILE_TEMPERATURE ||
    flags.ENABLE_PROFILE_SIGNAL_PLOT ||
    flags.ENABLE_PROFILE_PRESSURE ||
    flags.ENABLE_PROFILE_PRESSURE_PROFILE
  );
};