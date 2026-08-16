# Compile-Time Feature Flags

This document explains how to completely exclude features from the application bundle at build time. This is useful for creating specialized builds where certain functionalities are not needed, resulting in a smaller and more optimized application.

This method differs from the runtime feature flags in `cassandra-config.json`, which only hide UI elements. Compile-time flags ensure the underlying code is never included in the final build.

## How It Works

We use [Vite's environment variables](https://vitejs.dev/guide/env-and-mode.html) to control which features are included. During the build process, Vite replaces these environment variables with their actual values. Any code inside a conditional block that evaluates to `false` (e.g., `if (false) { ... }`) is considered "dead code" and is removed from the bundle through a process called tree-shaking.

## Configuration

1.  **Create an Environment File**: In the root of the `modbus-ui` directory, create a new file named `.env.local`. This file is ignored by Git and is used for local configuration overrides.

2.  **Define Feature Flags**: Add flags to your `.env.local` file, prefixing each with `VITE_`. By default, all features are enabled. To disable a feature and exclude it from the build, set its corresponding variable to `"false"`.

    **Example `.env.local`:**
    ```env
    # Disables the Signal Plot feature
    VITE_ENABLE_PROFILE_SIGNAL_PLOT="false"

    # Disables the VFD controls
    VITE_ENABLE_SAKO_VFD="false"
    ```

    Any flag not present in the `.env.local` file will be considered enabled.

## Available Flags

The following environment variables can be used to control the build:

| Environment Variable | Controlled Feature |
| :--- | :--- |
| `VITE_ENABLE_OMRON_E5` | Controller Partitions & Controller Chart |
| `VITE_ENABLE_AMPERAGE_BUDGET_MANAGER` | Sequential Heating |
| `VITE_ENABLE_PROFILE_TEMPERATURE` | Temperature Profiles |
| `VITE_ENABLE_PROFILE_SIGNAL_PLOT` | Signal Plot Editor |
| `VITE_ENABLE_PRESS_CYLINDER` | Press Cylinder Controls |
| `VITE_ENABLE_SAKO_VFD` | VFD Controls |

## Usage in Code

To make this system work, we would need to create a centralized `features.ts` file that reads both the compile-time environment variables and the runtime JSON configuration. The application components would then import flags from this file.

**Proposed `src/features.ts`:**
```typescript
import localConfig from './cassandra-config.json';

// Compile-time flags (from .env files)
const compileTimeFlags = {
  ENABLE_OMRON_E5: import.meta.env.VITE_ENABLE_OMRON_E5 !== 'false',
  ENABLE_AMPERAGE_BUDGET_MANAGER: import.meta.env.VITE_ENABLE_AMPERAGE_BUDGET_MANAGER !== 'false',
  ENABLE_PROFILE_TEMPERATURE: import.meta.env.VITE_ENABLE_PROFILE_TEMPERATURE !== 'false',
  ENABLE_PROFILE_SIGNAL_PLOT: import.meta.env.VITE_ENABLE_PROFILE_SIGNAL_PLOT !== 'false',
  ENABLE_PRESS_CYLINDER: import.meta.env.VITE_ENABLE_PRESS_CYLINDER !== 'false',
  ENABLE_SAKO_VFD: import.meta.env.VITE_ENABLE_SAKO_VFD !== 'false',
};

// The application would then use a merged configuration.
// For now, this file illustrates how compile-time flags would be read.
// The actual implementation would require further changes to the ModbusContext.
export const features = {
    ...localConfig.features,
    ...compileTimeFlags
};

```

By using this approach, if `VITE_ENABLE_PROFILE_SIGNAL_PLOT` is set to `"false"`, any code inside a block like `if (features.ENABLE_PROFILE_SIGNAL_PLOT) { ... }` will be removed from the final JavaScript bundle. 