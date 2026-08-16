
import React from 'react';
import { Link, useLocation } from 'react-router-dom';
import { cn } from '@/lib/utils';
import { Thermometer, LineChart, Settings, Moon, Sun, Grid } from 'lucide-react';
import { useTheme } from '@/components/ThemeProvider';

interface MainNavProps {
  className?: string;
}

const MainNav: React.FC<MainNavProps> = ({ className }) => {
  const location = useLocation();
  const { theme, setTheme } = useTheme();
  
  const navItems = [
    {
      name: 'Controllers',
      href: '/',
      icon: <Thermometer className="h-5 w-5" />,
      active: location.pathname === '/'
    },
    {
      name: 'Profiles',
      href: '/profiles',
      icon: <LineChart className="h-5 w-5" />,
      active: location.pathname === '/profiles'
    },
    {
      name: 'Zones',
      href: '/zone-settings',
      icon: <Grid className="h-5 w-5" />,
      active: location.pathname === '/zone-settings'
    },
    {
      name: 'Settings',
      href: '/settings',
      icon: <Settings className="h-5 w-5" />,
      active: location.pathname === '/settings'
    }
  ];
  
  const toggleTheme = () => {
    setTheme(theme === 'light' ? 'dark' : 'light');
  };
  
  return (
    <nav className={cn("flex justify-between items-center", className)}>
      <div className="flex space-x-2 rounded-lg overflow-hidden border border-border">
        {navItems.map((item) => (
          <Link
            key={item.href}
            to={item.href}
            className={cn(
              "flex items-center gap-2 px-3 py-2 text-sm transition-colors",
              item.active 
                ? "bg-primary text-primary-foreground" 
                : "hover:bg-muted"
            )}
          >
            {item.icon}
            <span>{item.name}</span>
          </Link>
        ))}
      </div>
      
      <button
        onClick={toggleTheme}
        className="p-2 rounded-full hover:bg-muted transition-colors"
        aria-label={`Switch to ${theme === 'light' ? 'dark' : 'light'} mode`}
      >
        {theme === 'light' ? (
          <Moon className="h-5 w-5" />
        ) : (
          <Sun className="h-5 w-5" />
        )}
      </button>
    </nav>
  );
};

export default MainNav;
