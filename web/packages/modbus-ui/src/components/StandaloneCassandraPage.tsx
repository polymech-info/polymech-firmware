import { Link, Outlet, useLocation } from 'react-router-dom';
import ConnectionPanel from './ConnectionPanel';
import { SlidersHorizontal, LineChart, Users, Settings2, Palette, Bug } from 'lucide-react';
import { T } from '../i18n';
import { cn } from '@/lib/utils';
import { useModbus } from '../contexts/ModbusContext';
import CollapsibleSection from './CollapsibleSection';
import ConnectionPanelHeader from './ConnectionPanelHeader';
import ConnectionManager from './ConnectionManager';
import { ExtensionSlot } from './layout/ExtensionSlot';
import { useState } from 'react';

const StandaloneCassandraPage = () => {
  const { featureFlags } = useModbus();
  const location = useLocation();
  const [isEditMode, setIsEditMode] = useState(false);

  const getTabValue = () => {
    if (location.pathname.startsWith('/advanced')) {
      return 'advanced';
    }
    if (location.pathname.startsWith('/debug')) {
      return 'debug';
    }
    switch (location.pathname) {
      case '/signals':
        return 'signals';
      case '/profiles':
        return 'profiles';
      case '/playground':
        return 'playground';
      case '/':
      default:
        return 'hmi';
    }
  };

  const activeTab = getTabValue();

  const handleEditModeToggle = (enabled: boolean) => {
    setIsEditMode(enabled);
  };

  // Determine current page ID for layout operations
  const getCurrentPageId = () => {
    switch (activeTab) {
      case 'playground':
        return 'playground';
      case 'hmi':
        return 'dashboard';
      case 'profiles':
        return 'profile';
      case 'signals':
        return 'signals';
      default:
        return 'dashboard';
    }
  };

  const tabStyle = "flex items-center justify-center px-4 py-2 text-sm font-medium transition-all focus:outline-none focus:ring-2 focus:ring-ring focus:ring-offset-2 text-slate-600 dark:text-slate-400 hover:bg-slate-100 dark:hover:bg-white/10 data-[state=active]:glass-card data-[state=active]:text-slate-700 dark:data-[state=active]:text-slate-100 data-[state=active]:shadow-lg";

  return (
    <div className="min-h-screen bg-background text-foreground">
      <ConnectionManager />
      <div className="max-w-4xl mx-auto space-y-4">
        <div className="flex justify-between items-start pt-4 md:pt-0">
          <div className="w-full">
            <CollapsibleSection
              title={<ConnectionPanelHeader onEditModeToggle={handleEditModeToggle} isEditMode={isEditMode} currentPageId={getCurrentPageId()} />}
              storageKey="standalone-connection-panel-open"
              initiallyOpen={true}
              className="glass-panel rounded-lg"
              headerClassName="flex justify-between items-center p-3 md:p-5"
              contentClassName="p-3 md:p-5 pt-0"
              titleClassName="w-full"
            >
              <ConnectionPanel />
            </CollapsibleSection>
          </div>          
        </div>

        {/* Extension Slot: After Connection Panel */}
        <ExtensionSlot 
          id="after-connection"
          currentPageId={getCurrentPageId()}
          isEditMode={isEditMode}
          canvasConfig={{ defaultColumns: 2, showTitle: true }}
        />

        <div className="w-full">
          <div className="flex flex-wrap w-full h-auto gap-3 md:grid md:grid-cols-5 glass-panel p-1 md:rounded-lg mx-0 sticky top-0 z-10">
            <Link to="/" data-state={activeTab === 'hmi' ? 'active' : 'inactive'} className={cn(tabStyle, "rounded-md")}>
              <SlidersHorizontal className="w-4 h-4 mr-1 sm:mr-2" /> <T>Dashboard</T> <span className="text-xs opacity-60"></span>
            </Link>
            {featureFlags.ENABLE_PROFILE_SIGNAL_PLOT && (
              <Link to="/signals" data-state={activeTab === 'signals' ? 'active' : 'inactive'} className={cn(tabStyle, "rounded-md")}>
                <LineChart className="w-4 h-4 mr-1 sm:mr-2" /> <T>Signals</T>
              </Link>
            )}
            {featureFlags.ENABLE_PROFILE_TEMPERATURE && (
              <Link to="/profiles" data-state={activeTab === 'profiles' ? 'active' : 'inactive'} className={cn(tabStyle, "rounded-md")}>
                <Users className="w-4 h-4 mr-1 sm:mr-2" /> <T>Profiles</T>
              </Link>
            )}
            <Link to="/playground" data-state={activeTab === 'playground' ? 'active' : 'inactive'} className={cn(tabStyle, "rounded-md")}>
              <Palette className="w-4 h-4 mr-1 sm:mr-2" /> <T>Playground</T>
            </Link>
            <Link to="/advanced" data-state={activeTab === 'advanced' ? 'active' : 'inactive'} className={cn(tabStyle, "rounded-md")}>
              <Settings2 className="w-4 h-4 mr-1 sm:mr-2" /> <T>Advanced</T>
            </Link>
          </div>
          <div className="mt-4 pt-4 p-0 md:p-4 glass-card md:rounded-lg">
            <Outlet context={{ isEditMode }} />
          </div>
        </div>

        {/* Extension Slot: Page Bottom */}
        <ExtensionSlot 
          currentPageId={getCurrentPageId()}
          id="page-bottom"
          isEditMode={isEditMode}
          canvasConfig={{ defaultColumns: 3, showTitle: true }}
        />
      </div>
    </div>
  );
};

export default StandaloneCassandraPage; 