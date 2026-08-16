import { z } from "zod";
import { networkSettingsSchema } from "./schemas/network-schema.js";
import { settingsSchema } from "./schemas/settings-schema.js";
import { profilesSchema } from "./schemas/profiles-schema.js";
import { signalPlotsSchema } from "./schemas/signal-plots-schema.js";

export type NetworkSettingsUpdatePayload = z.infer<
  typeof networkSettingsSchema
>;
export type SettingsUpdatePayload = z.infer<typeof settingsSchema>;
export type ProfilesUpdatePayload = z.infer<typeof profilesSchema>;
export type SignalPlotsUpdatePayload = z.infer<typeof signalPlotsSchema>; 