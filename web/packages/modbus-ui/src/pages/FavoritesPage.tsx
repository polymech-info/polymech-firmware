import CoilsDisplay from '@/components/CoilsDisplay';
import RegistersDisplay from '@/components/RegistersDisplay';
import { useFavorites } from '@/hooks/useFavorites';
import { T } from '@/i18n';

const FavoritesPage = () => {
  const { favoriteCoils, favoriteRegisters } = useFavorites();

  return (
    <div className="space-y-8">
      <div>
        <h2 className="text-2xl font-bold text-gradient mb-4"><T>Favorite Registers</T></h2>
        {favoriteRegisters.length > 0 ? (
          <RegistersDisplay showOnlyFavorites={true} />
        ) : (
          <div className="text-center py-10 rounded-lg bg-black/20">
            <p className="text-muted-foreground"><T>No favorite registers yet.</T></p>
          </div>
        )}
      </div>
      <div>
        <h2 className="text-2xl font-bold text-gradient mb-4"><T>Favorite Coils</T></h2>
        {favoriteCoils.length > 0 ? (
          <CoilsDisplay showOnlyFavorites={true} />
        ) : (
            <div className="text-center py-10 rounded-lg bg-black/20">
            <p className="text-muted-foreground"><T>No favorite coils yet.</T></p>
          </div>
        )}
      </div>
    </div>
  );
};

export default FavoritesPage; 