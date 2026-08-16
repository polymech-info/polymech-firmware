import React, { useState } from 'react';
import ConnectionPanel from './ConnectionPanel';
import DisplaySettings from './DisplaySettings';
import PlungerHMIDisplay from './PlungerHMIDisplay';
import PlungerSettingsDisplay from './PlungerSettingsDisplay';
import { Tabs, TabsContent, TabsList, TabsTrigger } from "@/components/ui/tabs";
import { SlidersHorizontal, Cog } from 'lucide-react'; // Icons for tabs
import NetworkSettingsDisplay from './NetworkSettingsDisplay';  
import { T } from '../i18n';

const StandalonePlungerPage = () => {
  const [activeTab, setActiveTab] = useState('hmi'); // 'hmi' or 'settings'

  return (
    <div className="min-h-screen bg-background text-foreground">
      <div className="max-w-4xl mx-auto space-y-4">
        {/* Top bar with ConnectionPanel and ThemeToggle */}
        <div className="flex justify-between items-start pt-4 px-4 md:pt-0 md:px-0">
          <div className="w-full">
              <ConnectionPanel />
          </div>          
        </div>

        <Tabs value={activeTab} onValueChange={setActiveTab} className="w-full">
          <TabsList className="grid w-full grid-cols-3 bg-background/50 border-b-0 md:border-b border-border backdrop-blur-sm sticky top-0 z-10 md:rounded-t-lg mx-0 md:mx-0">
            <TabsTrigger value="hmi" className="md:rounded-tl-md">
              <SlidersHorizontal className="w-4 h-4 mr-2" /> <T>Plunger HMI</T>
            </TabsTrigger>
            <TabsTrigger value="settings" className="md:rounded-tr-md">
              <Cog className="w-4 h-4 mr-2" /> <T>Plunger Settings</T>
            </TabsTrigger>
            <TabsTrigger value="network" className="md:rounded-tr-md">
              <Cog className="w-4 h-4 mr-2" /> <T>Network Settings</T>
            </TabsTrigger>
          </TabsList>
          <TabsContent value="hmi" className="mt-0 md:mt-0 p-0 md:p-4 border-x border-b border-border md:rounded-b-lg">
            <PlungerHMIDisplay />
          </TabsContent>
          <TabsContent value="settings" className="mt-0 md:mt-0 p-0 md:p-4 border-x border-b border-border md:rounded-b-lg">
            <PlungerSettingsDisplay />
          </TabsContent>
          <TabsContent value="network" className="mt-0 md:mt-0 p-0 md:p-4 border-x border-b border-border md:rounded-b-lg">
            <NetworkSettingsDisplay />
          </TabsContent>
        </Tabs>
      </div>
    </div>
  );
};

export default StandalonePlungerPage; 