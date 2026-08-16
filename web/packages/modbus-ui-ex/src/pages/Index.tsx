import { ModbusProvider } from '@/contexts/ModbusContext';
import ModbusUI from '@/components/ModbusUI';
import ThemeToggle from '@/components/ThemeToggle';
import { Brain, Cpu, Database, Server } from 'lucide-react';

const Index = () => {
  return (
    <div className="min-h-screen bg-gradient-to-br from-background via-background to-background/80 overflow-x-hidden">
      {/* Background tech elements */}
      <div className="fixed inset-0 overflow-hidden z-0">
        <div className="absolute top-20 left-10 text-white/5 dark:text-white/5 light:text-black/5 transform -rotate-12">
          <Brain size={200} />
        </div>
        <div className="absolute bottom-20 right-10 text-white/5 dark:text-white/5 light:text-black/5 transform rotate-12">
          <Cpu size={150} />
        </div>
        <div className="absolute top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 text-white/5 dark:text-white/5 light:text-black/5">
          <Server size={300} />
        </div>
        <div className="absolute bottom-10 left-20 text-white/5 dark:text-white/5 light:text-black/5">
          <Database size={100} />
        </div>
        
        {/* Data flow animation lines */}
        <div className="absolute inset-0 opacity-20">
          <div className="h-full w-full">
            {Array.from({ length: 20 }).map((_, i) => (
              <div 
                key={i}
                className="absolute h-[2px] w-[40px] bg-primary rounded-full animate-data-flow" 
                style={{ 
                  left: `${Math.random() * 100}%`, 
                  top: `${Math.random() * 100}%`,
                  opacity: 0.3 + Math.random() * 0.7,
                  animationDelay: `${Math.random() * 5}s`,
                  animationDuration: `${10 + Math.random() * 15}s`
                }}
              />
            ))}
          </div>
        </div>
      </div>
      
      {/* Header */}
      <header className="relative z-10 p-4 flex justify-between items-center">
        <div className="glass-morphism p-3 px-6 rounded-full">
          <h1 className="text-2xl font-bold text-gradient">
            Modbus Client <span className="text-primary glow-text">RTU/TCP</span>
          </h1>
        </div>
        <ThemeToggle />
      </header>
      
      {/* Main content */}
      <main className="relative z-10 w-full md:max-w-7xl md:mx-auto md:px-4 md:py-4">
        <ModbusProvider>
          <ModbusUI />
        </ModbusProvider>
      </main>
      
      {/* Footer */}
      <footer className="relative z-10 p-4 text-center text-xs text-muted-foreground">
        <p>ModbusTCP Client © {new Date().getFullYear()}</p>
      </footer>
    </div>
  );
};

export default Index;
