import React from 'react';

export interface WidgetMetadata {
  id: string;
  name: string;
  category: 'control' | 'display' | 'chart' | 'system' | 'custom';
  description: string;
  icon?: React.ComponentType;
  thumbnail?: string;
  defaultProps?: Record<string, any>;
  configSchema?: Record<string, any>;
  minSize?: { width: number; height: number };
  resizable?: boolean;
  tags?: string[];
}

export interface WidgetDefinition {
  component: React.ComponentType<any>;
  metadata: WidgetMetadata;
  previewComponent?: React.ComponentType<any>;
}

class WidgetRegistry {
  private widgets = new Map<string, WidgetDefinition>();

  register(definition: WidgetDefinition) {
    if (this.widgets.has(definition.metadata.id)) {
      console.warn(`Widget with id '${definition.metadata.id}' already registered`);
      return;
    }   
    this.widgets.set(definition.metadata.id, definition);
  }

  get(id: string): WidgetDefinition | undefined {
    return this.widgets.get(id);
  }

  getAll(): WidgetDefinition[] {
    return Array.from(this.widgets.values()).sort((a, b) => 
      a.metadata.name.localeCompare(b.metadata.name)
    );
  }

  getByCategory(category: string): WidgetDefinition[] {
    return this.getAll().filter(w => w.metadata.category === category);
  }

  search(query: string): WidgetDefinition[] {
    const lowercaseQuery = query.toLowerCase();
    return this.getAll().filter(w => 
      w.metadata.name.toLowerCase().includes(lowercaseQuery) ||
      w.metadata.description.toLowerCase().includes(lowercaseQuery) ||
      w.metadata.tags?.some(tag => tag.toLowerCase().includes(lowercaseQuery))
    );
  }

  clear() {
    this.widgets.clear();
  }

  getCount(): number {
    return this.widgets.size;
  }
}

export const widgetRegistry = new WidgetRegistry();
