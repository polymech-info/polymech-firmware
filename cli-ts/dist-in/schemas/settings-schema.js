import { z } from "zod";
const settingValueSchema = z.union([z.string(), z.number(), z.boolean()]);
const settingSchema = z.object({
    enabled: z.boolean(),
    id: z.number(),
    name: z.string(),
    group: z.string(),
    flags: z.number(),
    parent: z.number(),
    type: z.string(),
    value: settingValueSchema,
});
const controllerSchema = z.object({
    slaveid: z.number(),
    name: z.string(),
    enabled: z.boolean(),
});
const partitionSchema = z.object({
    name: z.string(),
    controllers: z.array(controllerSchema),
    startslaveid: z.number().optional(),
    numcontrollers: z.number().optional(),
});
export const settingsSchema = z.object({
    master: z.string(),
    slaves: z.array(z.string()),
    partitions: z.array(partitionSchema),
    settings: z.array(settingSchema),
});
//# sourceMappingURL=settings-schema.js.map