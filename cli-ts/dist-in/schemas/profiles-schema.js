import { z } from "zod";
export const controlPointSchema = z.object({
    x: z.number(),
    y: z.number(),
});
export const profileSchema = z.object({
    slot: z.number(),
    duration: z.number(),
    name: z.string(),
    max: z.number(),
    enabled: z.boolean(),
    signalPlot: z.number(),
    description: z.string(),
    controlPoints: z.array(controlPointSchema).optional(),
    targetRegisters: z.array(z.number()).optional(),
});
export const profilesSchema = profileSchema;
//# sourceMappingURL=profiles-schema.js.map