import { useState, useCallback, useEffect } from 'react';

const WATCHED_ITEMS_STORAGE_KEY = 'modbus_watched_items';

export interface WatchedItem {
  id: string;
  address: number;
  source: 'register' | 'coil';
  title: string;
}

const getStoredWatchedItems = (): WatchedItem[] => {
  try {
    const stored = localStorage.getItem(WATCHED_ITEMS_STORAGE_KEY);
    if (stored) {
      const parsed = JSON.parse(stored);
      if (Array.isArray(parsed)) {
        // Validate that each item has the required properties
        return parsed.filter(item => 
          item && 
          typeof item.id === 'string' &&
          typeof item.address === 'number' &&
          (item.source === 'register' || item.source === 'coil') &&
          typeof item.title === 'string'
        );
      }
    }
  } catch (error) {
    console.error("Failed to parse watched items from localStorage", error);
  }
  return [];
};

export const useWatchedItems = () => {
  const [watchedItems, setWatchedItems] = useState<WatchedItem[]>(getStoredWatchedItems);

  useEffect(() => {
    localStorage.setItem(WATCHED_ITEMS_STORAGE_KEY, JSON.stringify(watchedItems));
  }, [watchedItems]);

  const addWatchedItem = useCallback((address: number, source: 'register' | 'coil', title: string): boolean => {
    const id = `${source}-${address}`;
    
    // Check if item already exists
    const exists = watchedItems.some(item => item.id === id);
    if (exists) {
      return false; // Item already exists
    }

    const newItem: WatchedItem = {
      id,
      address,
      source,
      title
    };

    setWatchedItems(prev => [...prev, newItem]);
    return true; // Successfully added
  }, [watchedItems]);

  const removeWatchedItem = useCallback((id: string) => {
    setWatchedItems(prev => prev.filter(item => item.id !== id));
  }, []);

  const clearAllWatchedItems = useCallback(() => {
    setWatchedItems([]);
  }, []);

  const isWatched = useCallback((address: number, source: 'register' | 'coil'): boolean => {
    const id = `${source}-${address}`;
    return watchedItems.some(item => item.id === id);
  }, [watchedItems]);

  const getWatchedItem = useCallback((address: number, source: 'register' | 'coil'): WatchedItem | undefined => {
    const id = `${source}-${address}`;
    return watchedItems.find(item => item.id === id);
  }, [watchedItems]);

  const reorderWatchedItems = useCallback((fromIndex: number, toIndex: number) => {
    setWatchedItems(prev => {
      const newItems = [...prev];
      const [removed] = newItems.splice(fromIndex, 1);
      newItems.splice(toIndex, 0, removed);
      return newItems;
    });
  }, []);

  return {
    watchedItems,
    addWatchedItem,
    removeWatchedItem,
    clearAllWatchedItems,
    isWatched,
    getWatchedItem,
    reorderWatchedItems,
  };
};