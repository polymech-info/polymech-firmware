# Runtime Feature Loading (Code Splitting)

This document outlines a strategy for splitting the application's JavaScript bundle by feature, allowing parts of the application to be loaded on-demand at runtime. This practice, often called "code splitting," can significantly improve initial load performance by reducing the size of the main bundle that a user must download when they first visit the site.

## How It Works

We leverage Vite's built-in support for code splitting via dynamic `import()` statements, in combination with React's `lazy` and `Suspense` APIs.

1.  **Dynamic `import()`**: When Vite encounters a dynamic `import('...')` expression, it automatically creates a separate JavaScript "chunk" for that module. This chunk will not be included in the main application bundle.

2.  **`React.lazy`**: This function lets you render a dynamically imported component as a regular component. It takes a function that must call a dynamic `import()`. `React.lazy` returns a promise-like component that React can manage.

3.  **`React.Suspense`**: This component lets you specify a loading indicator (a "fallback" UI) if the component you're trying to render is not yet loaded. You can wrap your lazy-loaded components or routes in a `Suspense` boundary.

When a user navigates to a part of the application that requires a lazy-loaded component, React will suspend rendering, display the fallback UI, and automatically fetch the required JavaScript chunk from the server. Once the chunk is downloaded, React will render the component.

## Implementation Example

Here is how `src/App.tsx` could be refactored to implement this strategy. The main page components associated with features are loaded dynamically.

**Example `src/App.tsx` for Runtime Loading:**

```typescript
import React, { Suspense } from 'react';
import { HashRouter, Routes, Route, Navigate } from 'react-router-dom';
import { ModbusProvider, useModbus } from './contexts/ModbusContext';

// --- Eagerly-loaded core components ---
import ModbusUI from './components/ModbusUI';
import StandaloneCassandraPage from './components/StandaloneCassandraPage';
import { Toaster } from '@/components/ui/sonner';
import CassandraHMIDisplay from './components/CassandraHMIDisplay';
import NetworkSettingsDisplay from './components/NetworkSettingsDisplay';
import CoilsPage from './pages/CoilsPage';
import RegistersPage from './pages/RegistersPage';
import LogsPage from './pages/LogsPage';
import AdvancedPage from './pages/AdvancedPage';
import LoadingSpinner from './components/ui/LoadingSpinner'; // Assuming a loading component exists

// --- Lazy-loaded feature components ---
const ProfilePage = React.lazy(() => import('./pages/profiles/ProfilePage'));
const SignalPlotEditor = React.lazy(() => import('./components/SignalPlotEditor'));
const ProfilesDisplay = React.lazy(() => import('./components/ProfilesDisplay'));
const CassandraSettingsDisplay = React.lazy(() => import('./components/CassandraSettingsDisplay'));

// --- Main routing logic ---
const AppRoutes = () => {
  const { featureFlags } = useModbus();

  return (
    <Routes>
      {/* Core routes that are always present */}
      <Route path="/dashboard" element={<ModbusUI />} />
      
      {/* Feature-gated routes with lazy-loaded components */}
      {featureFlags.ENABLE_PROFILE_TEMPERATURE && (
        <>
          <Route path="/profiles/new" element={<ProfilePage />} />
          <Route path="/profiles/edit/:slot" element={<ProfilePage />} />
        </>
      )}
      {featureFlags.ENABLE_PROFILE_SIGNAL_PLOT && (
        <Route path="/signal-plot-editor" element={<SignalPlotEditor />} />
      )}
      
      <Route path="/" element={<StandaloneCassandraPage />}>
        <Route index element={<CassandraHMIDisplay />} />
        {featureFlags.ENABLE_PROFILE_SIGNAL_PLOT && <Route path="signals" element={<SignalPlotEditor />} />}
        {featureFlags.ENABLE_PROFILE_TEMPERATURE && <Route path="profiles" element={<ProfilesDisplay />} />}
        <Route path="advanced" element={<AdvancedPage />}>
          <Route index element={<Navigate to="settings" replace />} />
          {/* Settings are also lazy-loaded as they can be complex */}
          <Route path="settings" element={<CassandraSettingsDisplay />} />
          <Route path="network" element={<NetworkSettingsDisplay />} />
          <Route path="coils" element={<CoilsPage />} />
          <Route path="registers" element={<RegistersPage />} />
          <Route path="logs" element={<LogsPage />} />
        </Route>
      </Route>
      
      <Route path="*" element={<Navigate to="/" replace />} />
    </Routes>
  );
};

// --- Main App component with Suspense Boundary ---
const App = () => {
  return (
    <HashRouter>
      <ModbusProvider>
        {/*
          The Suspense fallback will be shown whenever a lazy-loaded 
          component is being fetched. You can place multiple Suspense 
          boundaries at different levels of the component tree to create
          a more granular loading experience.
        */}
        <Suspense fallback={<div className="flex justify-center items-center h-screen"><LoadingSpinner /></div>}>
          <AppRoutes />
          <Toaster />
        </Suspense>
      </ModbusProvider>
    </HashRouter>
  );
};

export default App;
```

By applying this pattern, the initial download for the user is smaller and faster, and the code for features like "Profiles" or "Signal Plots" is only loaded if and when the user accesses them. 