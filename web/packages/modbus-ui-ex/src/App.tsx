import React from 'react';
import { HashRouter, Routes, Route, Navigate } from 'react-router-dom';
import { ModbusProvider } from './contexts/ModbusContext';
import ModbusUI from './components/ModbusUI';
import StandalonePlungerPage from './components/StandalonePlungerPage';
import StandaloneCassandraPage from './components/StandaloneCassandraPage';
import SignalPlotEditor from './components/SignalPlotEditor';
import ProfilePage from './pages/profiles/ProfilePage';
import { Toaster } from '@/components/ui/sonner';
import CassandraHMIDisplay from './components/CassandraHMIDisplay';
import CassandraSettingsDisplay from './components/CassandraSettingsDisplay';
import NetworkSettingsDisplay from './components/NetworkSettingsDisplay';
import ProfilesDisplay from './components/ProfilesDisplay';

const App = () => {
  return (
    <HashRouter>
      <ModbusProvider>
        <Routes>
          <Route 
            path="/dashboard"
            element={
              <div className="min-h-screen bg-background text-foreground">
                <main className="container mx-auto">
                  <ModbusUI />
                </main>
                <Toaster />
              </div>
            } 
          />
          <Route 
            path="/profiles/new" 
            element={<ProfilePage />} 
          />
          <Route 
            path="/profiles/edit/:slot" 
            element={<ProfilePage />} 
          />
          <Route 
            path="/signal-plot-editor"
            element={
              <div className="min-h-screen bg-background text-foreground p-4">
                <SignalPlotEditor />
              </div>
            }
          />
          <Route 
            path="/"
            element={
              <div className="min-h-screen bg-background text-foreground p-2 sm:p-4 space-y-4">
                <StandaloneCassandraPage />
              </div>
            }
          >
            <Route index element={<CassandraHMIDisplay />} />
            <Route path="settings" element={<CassandraSettingsDisplay />} />
            <Route path="network" element={<NetworkSettingsDisplay />} />
            <Route path="signals" element={<SignalPlotEditor />} />
            <Route path="profiles" element={<ProfilesDisplay />} />
          </Route>
          <Route path="*" element={<Navigate to="/" replace />} />
        </Routes>
      </ModbusProvider>
    </HashRouter>
  );
};

export default App;
