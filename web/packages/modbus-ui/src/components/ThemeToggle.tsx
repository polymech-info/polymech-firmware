
import { useState, useEffect } from 'react';
import { Toggle } from '@/components/ui/toggle';
import { Sun, Moon } from 'lucide-react';
import { toast } from '@/hooks/use-toast';

const ThemeToggle = () => {
  const [isDark, setIsDark] = useState(true);

  useEffect(() => {
    // Set initial theme based on localStorage or default to dark
    const savedTheme = localStorage.getItem('theme');
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    
    if (savedTheme === 'light') {
      setIsDark(false);
      document.documentElement.classList.remove('dark');
      document.documentElement.classList.add('light');
    } else if (savedTheme === 'dark' || prefersDark) {
      setIsDark(true);
      document.documentElement.classList.add('dark');
      document.documentElement.classList.remove('light');
    }
  }, []);

  const toggleTheme = () => {
    if (isDark) {
      document.documentElement.classList.remove('dark');
      document.documentElement.classList.add('light');
      localStorage.setItem('theme', 'light');
      toast({
        title: "Light theme activated"
      });
    } else {
      document.documentElement.classList.add('dark');
      document.documentElement.classList.remove('light');
      localStorage.setItem('theme', 'dark');
      toast({
        title: "Dark theme activated"
      });
    }
    setIsDark(!isDark);
  };

  return (
    <div onClick={(e) => e.stopPropagation()}>
      <Toggle
        variant="outline"
        size="icon"
        aria-label="Toggle theme"
        pressed={isDark}
        onPressedChange={toggleTheme}
        className="rounded-full"
      >
        {isDark ? (
          <Moon className="h-5 w-5" />
        ) : (
          <Sun className="h-5 w-5" />
        )}
      </Toggle>
    </div>
  );
};

export default ThemeToggle;
