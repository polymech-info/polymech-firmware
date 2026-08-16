import { z } from "zod";

export const signalTypeEnum = z.enum([
  "NONE",
  "MB_WRITE_COIL",
  "MB_WRITE_HOLDING_REGISTER",
  "CALL_METHOD",
  "CALL_FUNCTION",
  "CALL_REST",
  "GPIO_WRITE",
  "DISPLAY_MESSAGE",
  "USER_DEFINED",
  "PAUSE_PROFILE",
]);

export const signalStateEnum = z.enum([
  "STATE_NONE",
  "STATE_ERROR",
  "STATE_ON",
  "STATE_OFF",
  "STATE_CUSTOM_1",
]);

export const signalControlPointSchema = z.object({
  id: z.number(),
  time: z.number(),
  state: z.number(),
  type: z.number(),
  arg_0: z.number(),
  arg_1: z.number(),
  arg_2: z.number().optional(),
  user: z.any().optional(),
  name: z.string().optional(),
  description: z.string().optional(),
});

export const signalPlotSchema = z.object({
  name: z.string(),
  duration: z.number(),
  slot: z.number(),
  controlPoints: z.array(signalControlPointSchema),
});

export const signalPlotsSchema = z.array(signalPlotSchema); 