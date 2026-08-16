import React from 'react';
import { Link, Outlet, useLocation } from 'react-router-dom';
import ConnectionPanel from './ConnectionPanel';
import DisplaySettings from './DisplaySettings';
import CassandraHMIDisplay from './CassandraHMIDisplay';
import CassandraSettingsDisplay from './CassandraSettingsDisplay';
import ProfilesDisplay from './ProfilesDisplay';
import SignalPlotEditor from './SignalPlotEditor';
import { SlidersHorizontal, Cog, BarChartHorizontalBig } from 'lucide-react';
import NetworkSettingsDisplay from './NetworkSettingsDisplay';
import { T } from '../i18n';
import { cn } from '@/lib/utils';

const StandaloneCassandraPage = () => {
  const location = useLocation();
  const getTabValue = () => {
    switch (location.pathname) {
      case '/settings':
        return 'settings';
      case '/network':
        return 'network';
      case '/signals':
        return 'signals';
      case '/profiles':
        return 'profiles';
      case '/':
      default:
        return 'hmi';
    }
  };

  const activeTab = getTabValue();

  const tabStyle = "flex items-center justify-center px-4 py-2 text-sm font-medium transition-colors border border-border data-[state=active]:bg-primary data-[state=active]:text-primary-foreground data-[state=active]:shadow hover:bg-muted/50 focus:outline-none focus:ring-2 focus:ring-ring focus:ring-offset-2";

  return (
    <div className="min-h-screen bg-background text-foreground">
      <div className="max-w-4xl mx-auto space-y-4">
        <div className="flex justify-between items-start pt-4 md:pt-0">
          <div className="w-full">
              <ConnectionPanel />
          </div>          
        </div>

        <div className="w-full">
          <div className="flex flex-wrap w-full h-auto gap-3 md:grid md:grid-cols-5 bg-background border-b-0 md:border-b border-border md:rounded-t-lg mx-0 sticky top-0 z-10">
            <Link to="/" data-state={activeTab === 'hmi' ? 'active' : 'inactive'} className={cn(tabStyle, "md:rounded-tl-md")}>
              <SlidersHorizontal className="w-4 h-4 mr-1 sm:mr-2" /> <T>Dashboard</T> 
            </Link>
            <Link to="/settings" data-state={activeTab === 'settings' ? 'active' : 'inactive'} className={tabStyle}>
              <Cog className="w-4 h-4 mr-1 sm:mr-2" /> <T>Settings</T>
            </Link>
            <Link to="/network" data-state={activeTab === 'network' ? 'active' : 'inactive'} className={tabStyle}>
              <Cog className="w-4 h-4 mr-1 sm:mr-2" /> <T>Wifi</T>
            </Link>
            <Link to="/signals" data-state={activeTab === 'signals' ? 'active' : 'inactive'} className={tabStyle}>
              <BarChartHorizontalBig className="w-4 h-4 mr-1 sm:mr-2" /> <T>Signals</T>
            </Link>
            <Link to="/profiles" data-state={activeTab === 'profiles' ? 'active' : 'inactive'} className={cn(tabStyle, "md:rounded-tr-md")}>
              <Cog className="w-4 h-4 mr-1 sm:mr-2" /> <T>Profiles</T>
            </Link>
          </div>
          <div className="mt-0 md:mt-0 pt-24 md:pt-10 p-0 md:px-4 md:pb-4 md:border-x md:border-b md:border-border md:rounded-b-lg">
            <Outlet />
          </div>
        </div>
      </div>
    </div>
  );
};

export default StandaloneCassandraPage; 