# Application Feature Flags

This document outlines how to enable or disable specific features and UI components within the Cassandra application by modifying the `src/cassandra-config.json` file.

## Configuration File

Feature flags are managed in `src/cassandra-config.json`. To enable or disable a feature, simply change its corresponding boolean value to `true` or `false`.

```json
{
  "features": {
    "ENABLE_OMRON_E5": true,
    "ENABLE_AMPERAGE_BUDGET_MANAGER": true,
    "ENABLE_PROFILE_TEMPERATURE": true,
    "ENABLE_PROFILE_SIGNAL_PLOT": true,
    "ENABLE_PRESS_CYLINDER": true,
    // ... other flags
  }
}
```

## Feature Mappings

The following table details which features are controlled by which flags and where they appear in the application.

| Feature Flag | UI Feature / Component | Controlled Routes / Areas |
| :--- | :--- | :--- |
| `ENABLE_OMRON_E5` | Controller Partitions | A section within the settings page at `/advanced/settings`. |
| `ENABLE_AMPERAGE_BUDGET_MANAGER` | Sequential Heating | A section within the settings page at `/advanced/settings`. |
| `ENABLE_PROFILE_TEMPERATURE` | Temperature Profiles | The main profile creation and editing pages at `/profiles`, `/profiles/new`, and `/profiles/edit/:slot`. |
| `ENABLE_PROFILE_SIGNAL_PLOT` | Signal Plot Editor | The signal plot editor pages at `/signals` and `/signal-plot-editor`. |
| `ENABLE_PRESS_CYLINDER` | Press Cylinder Controls | The press cylinder control panel on the main HMI display. |


## Implementation Example (`App.tsx`)

The application routes are conditionally rendered based on these flags. For example, in `src/App.tsx`, the routes for creating and editing temperature profiles would be wrapped in a condition like this:

```tsx
import features from './cassandra-config.json';

//...

<Routes>
  {/* ... other routes */}

  {features.ENABLE_PROFILE_TEMPERATURE && (
    <>
      <Route path="/profiles/new" element={<ProfilePage />} />
      <Route path="/profiles/edit/:slot" element={<ProfilePage />} />
    </>
  )}
  
  <Route path="/" element={<StandaloneCassandraPage />}>
    <Route index element={<CassandraHMIDisplay />} />
    
    {features.ENABLE_PROFILE_SIGNAL_PLOT && (
        <Route path="signals" element={<SignalPlotEditor />} />
    )}

    {features.ENABLE_PROFILE_TEMPERATURE && (
        <Route path="profiles" element={<ProfilesDisplay />} />
    )}
    
    {/* ... other nested routes */}
  </Route>

  {/* ... */}
</Routes>
``` 