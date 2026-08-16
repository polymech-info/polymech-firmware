
// Base Storage Service is now a minimal class since we're using Supabase
class BaseStorageService {
  // This class serves as an abstract base for our storage services
  // The actual implementation is delegated to Supabase in the concrete classes
}

export const baseStorageService = new BaseStorageService();
