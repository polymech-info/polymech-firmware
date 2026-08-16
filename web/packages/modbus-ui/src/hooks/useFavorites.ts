import { useState, useCallback, useEffect } from 'react';

const FAVORITES_STORAGE_KEY = 'modbus_favorites';

type FavoriteType = 'coil' | 'register';

interface Favorites {
  coils: number[];
  registers: number[];
}

const getStoredFavorites = (): Favorites => {
  try {
    const stored = localStorage.getItem(FAVORITES_STORAGE_KEY);
    if (stored) {
      const parsed = JSON.parse(stored);
      if (Array.isArray(parsed.coils) && Array.isArray(parsed.registers)) {
        return parsed;
      }
    }
  } catch (error) {
    console.error("Failed to parse favorites from localStorage", error);
  }
  return { coils: [], registers: [] };
};

export const useFavorites = () => {
  const [favorites, setFavorites] = useState<Favorites>(getStoredFavorites);

  useEffect(() => {
    localStorage.setItem(FAVORITES_STORAGE_KEY, JSON.stringify(favorites));
  }, [favorites]);

  // Listen for localStorage changes from other tabs/instances
  useEffect(() => {
    const handleStorageChange = (e: StorageEvent) => {
      if (e.key === FAVORITES_STORAGE_KEY && e.newValue) {
        try {
          const newFavorites = JSON.parse(e.newValue);
          setFavorites(newFavorites);
        } catch (error) {
          console.error('Failed to parse favorites from storage event', error);
        }
      }
    };

    window.addEventListener('storage', handleStorageChange);
    return () => window.removeEventListener('storage', handleStorageChange);
  }, []);

  const isFavorite = useCallback((type: FavoriteType, address: number): boolean => {
    if (type === 'coil') {
      return favorites.coils.includes(address);
    }
    return favorites.registers.includes(address);
  }, [favorites]);

  const toggleFavorite = useCallback((type: FavoriteType, address: number) => {
    // Get fresh data from localStorage to avoid stale state issues
    const freshFavorites = getStoredFavorites();
    
    setFavorites(prev => {
      const currentList = type === 'coil' ? freshFavorites.coils : freshFavorites.registers;
      const exists = currentList.includes(address);
      
      if (type === 'coil') {
        const newCoils = exists 
          ? currentList.filter(fav => fav !== address)
          : [...currentList, address].sort((a, b) => a - b);
        return { ...freshFavorites, coils: newCoils };
      } else {
        const newRegisters = exists 
          ? currentList.filter(fav => fav !== address)
          : [...currentList, address].sort((a, b) => a - b);
        return { ...freshFavorites, registers: newRegisters };
      }
    });
  }, []);
  
  return {
    favoriteCoils: favorites.coils,
    favoriteRegisters: favorites.registers,
    isFavorite,
    toggleFavorite,
  };
}; 