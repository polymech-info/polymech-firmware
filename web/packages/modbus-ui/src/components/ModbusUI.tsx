import React, { useState } from 'react';
import ConnectionPanel from './ConnectionPanel';
import CoilsDisplay from './CoilsDisplay';
import RegistersDisplay from './RegistersDisplay';
import LogViewer from './LogViewer';
import NetworkSettingsDisplay from './NetworkSettingsDisplay';
import PlungerSettingsDisplay from './PlungerSettingsDisplay';
import CassandraHMIDisplay from './CassandraHMIDisplay';
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { List, BarChartHorizontalBig, Server, Settings, SlidersHorizontal } from 'lucide-react';
import { T } from '../i18n'; // Assuming T component is used for i18n
import { Card, CardContent } from './ui/card';

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
        <TabsContent value="cassandra" className="mt-0">
          <Card className="rounded-t-none border-t-0">
            <CardContent className="pt-6">
              <Suspense fallback={<div>Loading...</div>}>
                <CassandraHMIDisplay />
              </Suspense>
            </CardContent>
          </Card>
        </TabsContent>
        <TabsContent value="coils" className="mt-0">
          <Card className="rounded-t-none border-t-0">
            <CardContent className="pt-6">
              <CoilsDisplay />
            </CardContent>
          </Card>
        </TabsContent>
        <TabsContent value="registers" className="mt-0">
          <Card className="rounded-t-none border-t-0">
            <CardContent className="pt-6">
              <RegistersDisplay />
            </CardContent>
          </Card>
        </TabsContent>
        <TabsContent value="logs" className="mt-0">
          <Card className="rounded-t-none border-t-0">
            <CardContent className="pt-6">
              <LogViewer />
            </CardContent>
          </Card>
        </TabsContent>
        <TabsContent value="settings" className="mt-0">
          <Card className="rounded-t-none border-t-0">
            <CardContent className="pt-6 space-y-6">
              <div>
                <Suspense fallback={<div>Loading...</div>}>
                  <NetworkSettingsDisplay />
                </Suspense>
              </div>
              <hr className="my-4 border-border" />
              <div>
                <Suspense fallback={<div>Loading...</div>}>
                  <PlungerSettingsDisplay />
                </Suspense>
              </div>
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </div>
  );
};

export default ModbusUI;
