import React from 'react';
import LogViewer from '@/components/LogViewer';
import { T } from '@/i18n';
import ConnectionManager from '@/components/ConnectionManager';

const LogViewerPage: React.FC = () => {
  return (
    <div className="p-4 bg-background text-foreground min-h-screen flex flex-col">
      <div className="flex justify-between items-start mb-4 flex-shrink-0">
        <h1 className="text-2xl font-bold">
          <T>Log Viewer</T>
        </h1>
        <ConnectionManager />
      </div>
      <LogViewer />
    </div>
  );
};

export default LogViewerPage;
