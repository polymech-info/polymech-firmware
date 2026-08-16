import React, { Suspense } from 'react';
import { HashRouter } from 'react-router-dom';
import { ModbusProvider } from './contexts/ModbusContext';
import AppRoutes from './AppRoutes';

const App = () => {
  return (
    <HashRouter>
      <ModbusProvider>
        <Suspense fallback={<div>Loading...</div>}>
          <AppRoutes />
        </Suspense>
      </ModbusProvider>
    </HashRouter>
  );
};

export default App;
