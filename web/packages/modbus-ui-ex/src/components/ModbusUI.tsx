import React, { useState, useEffect } from 'react';
import ConnectionPanel from './ConnectionPanel';
import DisplaySettings from './DisplaySettings';
import CoilsDisplay from './CoilsDisplay';
import RegistersDisplay from './RegistersDisplay';
import LogViewer from './LogViewer';
import QueueDisplay from './QueueDisplay';
import SlaveDataDisplay from './SlaveDataDisplay';
import NetworkSettingsDisplay from './NetworkSettingsDisplay';
import PlungerSettingsDisplay from './PlungerSettingsDisplay';
import CassandraSettingsDisplay from './CassandraSettingsDisplay';
import CassandraHMIDisplay from './CassandraHMIDisplay';
import PlungerHMIDisplay from './PlungerHMIDisplay';
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { List, BarChartHorizontalBig, Server, Inbox, Database, Settings, SlidersHorizontal } from 'lucide-react';
import { T } from '../i18n'; // Assuming T component is used for i18n

const ModbusUI = () => {
  const [activeTab, setActiveTab] = useState('cassandra'); // Changed default to cassandra as it's first

  return (
    <div className="md:p-4 md:space-y-4"> {/* Consider removing md:p-4 if page wrapper handles it */}
      <ConnectionPanel />
      <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
        <TabsList className="flex flex-wrap w-full h-auto gap-3 md:grid md:grid-cols-5 bg-background border-b-0 md:border-b border-border md:rounded-t-lg mx-0 sticky top-0 z-10">
          <TabsTrigger value="cassandra" className="md:rounded-tl-md border border-border"> 
            <SlidersHorizontal className="w-4 h-4 mr-1 sm:mr-2" /> <T>Cassandra</T>
          </TabsTrigger>
          <TabsTrigger value="coils" className="border border-border">
            <BarChartHorizontalBig className="w-4 h-4 mr-1 sm:mr-2" /> <T>Coils</T>
          </TabsTrigger>
          <TabsTrigger value="registers" className="border border-border">
             <Server className="w-4 h-4 mr-1 sm:mr-2" /> <T>Registers</T>
          </TabsTrigger>
          <TabsTrigger value="logs" className="border border-border">
            <List className="w-4 h-4 mr-1 sm:mr-2" /> <T>Logs</T>
          </TabsTrigger>
          {/* Removed Plunger and specific settings tabs to match Cassandra example, assuming simplification */}
          {/* Add them back if needed, ensuring md:grid-cols-X matches total tabs */}
          <TabsTrigger value="settings" className="md:rounded-tr-md border border-border"> {/* Adjusted for 5 tabs example */}
            <Settings className="w-4 h-4 mr-1 sm:mr-2" /> <T>Settings</T>
          </TabsTrigger>
        </TabsList>
        <TabsContent value="cassandra" className="mt-0 md:mt-0 pt-2 md:pt-10 md:px-4 md:pb-4 md:border-x md:border-b md:border-border md:rounded-b-lg">
          <CassandraHMIDisplay /> {/* Removed isDashboardView prop */}
        </TabsContent>
        <TabsContent value="coils" className="mt-0 md:mt-0 pt-2 md:pt-10 md:px-4 md:pb-4 md:border-x md:border-b md:border-border md:rounded-b-lg">
          <CoilsDisplay />
        </TabsContent>
        <TabsContent value="registers" className="mt-0 md:mt-0 pt-2 md:pt-10 md:px-4 md:pb-4 md:border-x md:border-b md:border-border md:rounded-b-lg">
          <RegistersDisplay />
        </TabsContent>
        <TabsContent value="logs" className="mt-0 md:mt-0 pt-2 md:pt-10 md:px-4 md:pb-4 md:border-x md:border-b md:border-border md:rounded-b-lg">
          <LogViewer />
        </TabsContent>
        <TabsContent value="settings" className="mt-0 md:mt-0 pt-2 md:pt-10 md:px-4 md:pb-4 md:border-x md:border-b md:border-border md:rounded-b-lg space-y-6">
          <div>
            <NetworkSettingsDisplay />
          </div>
          {/* For PlungerSettingsDisplay, if it's a separate tab, it needs its own TabsTrigger and TabsContent */}
          {/* If it's part of the 'settings' tab, it should be structured within this TabsContent */}
           <hr className="my-4 border-border" />
           <div>
             <PlungerSettingsDisplay />
           </div>
        </TabsContent>
      </Tabs>
    </div>
  );
};

export default ModbusUI;
