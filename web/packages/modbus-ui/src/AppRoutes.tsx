import { Suspense, lazy, useMemo, useEffect } from 'react';
import { Routes, Route, Navigate } from 'react-router-dom';
import { useModbus } from './contexts/ModbusContext';

import StandaloneCassandraPage from './components/StandaloneCassandraPage';
import { Toaster } from '@/components/ui/sonner';

// Register all widgets
import { registerAllWidgets } from '@/lib/registerWidgets';
import { LayoutProvider } from '@/contexts/LayoutContext';

const PressureProfilePage = lazy(() => import('./pages/profiles/PressureProfilePage'));

// Lazy load HMI components dynamically
const loadHMIComponent = (hmiName: string) => {
  const hmiMap = {
    elena: () => import('./components/ElenaHMIDisplay'),
    cassandra: () => import('./components/CassandraHMIDisplay'),
  };
  return lazy(hmiMap[hmiName as keyof typeof hmiMap] || hmiMap.elena);
};
import NetworkSettingsDisplay from './components/NetworkSettingsDisplay';
import CoilsPage from './pages/CoilsPage';
import RegistersPage from './pages/RegistersPage';
import LogsPage from './pages/LogsPage';
import AdvancedPage from './pages/AdvancedPage';
import FavoritesPage from './pages/FavoritesPage';
import SettingsDisplay from './components/SettingsDisplay';
import ProfilePage from './pages/profiles/ProfilePage';
import ProfilesDisplay from './components/ProfilesDisplay';
import ChartsPage from './pages/ChartsPage';
import ChartViewerPage from './pages/ChartViewerPage';
import LogViewerPage from './pages/LogViewerPage';
import PlaygroundPage from './pages/PlaygroundPage';
// import SignalPlotEditor from './components/SignalPlotEditor';

const SignalPlotEditor = import.meta.env.VITE_ENABLE_PROFILE_SIGNAL_PLOT !== 'false'
  ? lazy(() => import('./components/SignalPlotEditor'))
  : () => null;

const AppRoutes = () => {
  const { featureFlags, serverSettings } = useModbus();

  // Initialize widget registry
  useEffect(() => {
    registerAllWidgets();
  }, []);

  // HMI selector - get from server settings, fallback to 'elena'
  const hmi = serverSettings?.name || 'cassandra';
  // const hmi = 'cassandra';
  const HMIComponent = useMemo(() => {
    return loadHMIComponent(hmi);
  }, [hmi]);

  return (
    <LayoutProvider>
      <Suspense fallback={<div>Loading...</div>}>
        <Routes>
          {featureFlags.ENABLE_PROFILE_TEMPERATURE && (
            <>
              <Route
                path="/profiles/new"
                element={<ProfilePage />}
              />
              <Route
                path="/profiles/edit/:slot"
                element={<ProfilePage />}
              />
            </>
          )}
          {featureFlags.ENABLE_PROFILE_PRESSURE && (
            <Route
              path="/pressure-profiles/edit/:slot"
              element={<PressureProfilePage />}
            />
          )}
          {featureFlags.ENABLE_PROFILE_SIGNAL_PLOT && (
            <Route
              path="/signal-plot-editor"
              element={
                <div className="min-h-screen bg-background text-foreground p-4">
                  <SignalPlotEditor />
                </div>
              }
            />
          )}

          <Route
            path="/"
            element={
              <div className="min-h-screen bg-gradient-to-br from-slate-900 via-slate-800 to-indigo-900 dark:from-slate-950 dark:via-slate-900 dark:to-indigo-950 text-foreground sm:p-4 space-y-4 ">
                <StandaloneCassandraPage />
                <Toaster />
              </div>
            }
          >
            <Route index element={<HMIComponent />} />
            {featureFlags.ENABLE_PROFILE_SIGNAL_PLOT && <Route path="signals" element={<SignalPlotEditor />} />}
            {featureFlags.ENABLE_PROFILE_TEMPERATURE && <Route path="profiles" element={<ProfilesDisplay />} />}
            <Route path="playground" element={<PlaygroundPage />} />
            <Route path="advanced" element={<AdvancedPage />}>
              <Route path="settings" element={<SettingsDisplay />} />
              <Route path="network" element={<NetworkSettingsDisplay />} />
              <Route path="coils" element={<CoilsPage />} />
              <Route path="registers" element={<RegistersPage />} />
              <Route path="charts" element={<ChartsPage />} />
              <Route path="logs" element={<LogsPage />} />
              <Route path="favorites" element={<FavoritesPage />} />
            </Route>
          </Route>
          <Route path="/chart-viewer" element={<ChartViewerPage />} />
          <Route path="/log-viewer" element={<LogViewerPage />} />
          <Route path="*" element={<Navigate to="/" replace />} />
        </Routes>
      </Suspense>
    </LayoutProvider>
  );
};

export default AppRoutes;