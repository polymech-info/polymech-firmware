import { useEffect, useRef } from 'react';
import { useModbus } from '@/contexts/ModbusContext';

const ConnectionManager = () => {
  const {
    isConnected,
    connecting,
    connectionAborted,
    connectToServer,
    apiUrl,
  } = useModbus();

  const reconnectIntervalRef = useRef<NodeJS.Timeout | null>(null);
  const initialConnectionAttempted = useRef(false);
  const connectToServerRef = useRef(connectToServer);

  useEffect(() => {
    connectToServerRef.current = connectToServer;
  }, [connectToServer]);

  // Effect for initial connection attempt
  useEffect(() => {
    if (apiUrl && !initialConnectionAttempted.current) {
      connectToServerRef.current();
      initialConnectionAttempted.current = true;
    }
  }, [apiUrl]);

  // Effect for auto-reconnect
  useEffect(() => {
    const clearReconnectInterval = () => {
      if (reconnectIntervalRef.current) {
        clearInterval(reconnectIntervalRef.current);
        reconnectIntervalRef.current = null;
      }
    };

    if (!isConnected && !connecting && !connectionAborted) {
      clearReconnectInterval();
      reconnectIntervalRef.current = setInterval(() => {
        if (apiUrl) {
          connectToServerRef.current();
        }
      }, 5000);
    } else {
      clearReconnectInterval();
    }

    return () => {
      clearReconnectInterval();
    };
  }, [isConnected, connecting, connectionAborted, apiUrl]);

  return null; // This is a logic-only component
};

export default ConnectionManager; 