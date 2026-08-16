import React, { useState, useEffect } from 'react';
import { createPortal } from 'react-dom';
import { widgetRegistry } from '@/lib/widgetRegistry';
import { Input } from '@/components/ui/input';
import { Button } from '@/components/ui/button';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { Search, Plus, X } from 'lucide-react';
import { T } from '@/i18n';

interface WidgetPaletteProps {
  isVisible: boolean;
  onClose: () => void;
  onWidgetAdd: (widgetId: string) => void;
}

export const WidgetPalette: React.FC<WidgetPaletteProps> = ({ 
  isVisible, 
  onClose, 
  onWidgetAdd 
}) => {
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedCategory, setSelectedCategory] = useState<string>('all');

  // Handle Escape key
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'Escape' && isVisible) {
        onClose();
      }
    };

    if (isVisible) {
      document.addEventListener('keydown', handleKeyDown);
      // Focus the search input when opened
      setTimeout(() => {
        const searchInput = document.querySelector('#widget-search-input') as HTMLInputElement;
        if (searchInput) {
          searchInput.focus();
        }
      }, 100);
    }

    return () => {
      document.removeEventListener('keydown', handleKeyDown);
    };
  }, [isVisible, onClose]);

  if (!isVisible) return null;

  const widgets = searchQuery 
    ? widgetRegistry.search(searchQuery)
    : selectedCategory === 'all' 
      ? widgetRegistry.getAll()
      : widgetRegistry.getByCategory(selectedCategory);

  const categories = ['all', 'control', 'display', 'chart', 'system', 'custom'];

  const modalContent = (
    <div 
      className="fixed inset-0 bg-black/50 flex items-center justify-center z-[99999]" 
      onClick={onClose}
      style={{ zIndex: 99999 }}
    >
      <Card 
        className="glass-card w-96 max-w-[90vw] max-h-[80vh] overflow-hidden shadow-2xl"
        onClick={(e) => e.stopPropagation()}
        style={{ zIndex: 100000 }}
      >
        <CardHeader className="flex flex-row items-center justify-between pb-3">
          <CardTitle className="text-lg"><T>Add Widget</T></CardTitle>
          <Button
            variant="ghost"
            size="icon"
            onClick={onClose}
            className="h-8 w-8"
          >
            <X className="h-4 w-4" />
          </Button>
        </CardHeader>
        
        <CardContent className="space-y-4">
          {/* Search */}
          <div className="relative">
            <Search className="absolute left-2 top-1/2 transform -translate-y-1/2 h-4 w-4 text-slate-500" />
            <Input
              id="widget-search-input"
              placeholder="Search widgets..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="pl-8 glass-input"
            />
          </div>
          
          {/* Categories */}
          <div className="flex flex-wrap gap-1">
            {categories.map(category => (
              <Button
                key={category}
                size="sm"
                variant={selectedCategory === category ? "default" : "outline"}
                onClick={() => setSelectedCategory(category)}
                className="text-xs h-7"
              >
                {category.charAt(0).toUpperCase() + category.slice(1)}
              </Button>
            ))}
          </div>

          {/* Widget List */}
          <div className="space-y-2 max-h-64 overflow-y-auto">
            {widgets.map(widget => (
              <div
                key={widget.metadata.id}
                className="flex items-center justify-between p-3 border border-slate-300/30 dark:border-white/10 rounded-lg bg-white/5 dark:bg-black/5 hover:bg-white/10 dark:hover:bg-black/10 transition-colors cursor-pointer"
                onClick={() => {
                  onWidgetAdd(widget.metadata.id);
                  onClose();
                }}
              >
                <div className="flex items-center space-x-3 flex-1 min-w-0">
                  {widget.metadata.icon && (
                    <div className="h-5 w-5 text-slate-600 dark:text-slate-300 shrink-0">
                      {React.createElement(widget.metadata.icon, {})}
                    </div>
                  )}
                  <div className="min-w-0 flex-1">
                    <div className="font-medium text-sm truncate">{widget.metadata.name}</div>
                    <div className="text-xs text-slate-500 dark:text-slate-400 truncate">
                      {widget.metadata.description}
                    </div>
                  </div>
                </div>
                
                <Button
                  size="icon"
                  onClick={(e) => {
                    e.stopPropagation(); // Prevent the div's onClick from firing
                    onWidgetAdd(widget.metadata.id);
                    onClose();
                  }}
                  className="h-8 w-8 glass-button shrink-0"
                  title={`Add ${widget.metadata.name}`}
                >
                  <Plus className="h-4 w-4" />
                </Button>
              </div>
            ))}
            
            {widgets.length === 0 && (
              <div className="text-center text-slate-500 dark:text-slate-400 py-8">
                <T>No widgets found</T>
              </div>
            )}
          </div>
        </CardContent>
      </Card>
    </div>
  );

  // Render in portal to ensure it's at the top level
  return createPortal(modalContent, document.body);
};
