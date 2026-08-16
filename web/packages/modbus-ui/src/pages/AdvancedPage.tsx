import React, { useEffect } from 'react';
import { NavLink, Outlet, useLocation, useNavigate } from 'react-router-dom';
import { T } from '@/i18n';
import { cn } from '@/lib/utils';
import { Cog, Wifi, BarChartHorizontalBig, Server, List, Star, LineChart, Bug } from 'lucide-react';

const navLinks = [
    { to: 'settings', label: 'Settings', icon: Cog },
    { to: 'network', label: 'Network', icon: Wifi },
    { to: 'coils', label: 'Coils', icon: BarChartHorizontalBig },
    { to: 'registers', label: 'Registers', icon: Server },
    { to: 'charts', label: 'Charts', icon: LineChart },
    { to: 'logs', label: 'Logs', icon: List },
    { to: 'favorites', label: 'Favorites', icon: Star },
];

const AdvancedPage = () => {
    const location = useLocation();
    const navigate = useNavigate();

    useEffect(() => {
        const path = location.pathname.endsWith('/') ? location.pathname.slice(0, -1) : location.pathname;
        if (path === '/advanced') {
            const lastTab = localStorage.getItem('advanced_last_tab') || 'settings';
            navigate(lastTab, { replace: true });
        }
    }, [location.pathname, navigate]);

    const handleTabClick = (tab: string) => {
        localStorage.setItem('advanced_last_tab', tab);
    };
    
    return (
        <div className="space-y-4 p-4">
            <nav className="flex flex-wrap items-center gap-2 border-b border-border pb-2">
                {navLinks.map(link => (
                    <NavLink
                        key={link.to}
                        to={link.to}
                        onClick={() => handleTabClick(link.to)}
                        className={({ isActive }) => cn(
                            "flex items-center gap-2 rounded-md px-3 py-2 text-sm font-medium transition-colors",
                            isActive 
                                ? "bg-primary text-primary-foreground" 
                                : "text-muted-foreground hover:bg-muted/50 hover:text-foreground"
                        )}
                    >
                        <link.icon className="h-4 w-4" />
                        <T>{link.label}</T>
                    </NavLink>
                ))}
            </nav>
            <div className="mt-4">
                <Outlet />
            </div>
        </div>
    );
};

export default AdvancedPage; 