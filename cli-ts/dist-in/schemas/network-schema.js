import { z } from 'zod';
export const networkSettingsSchema = z.object({
    sta_ssid: z.string().optional(),
    sta_password: z.string().optional(),
    sta_local_ip: z.string().ip({ version: 'v4' }).optional(),
    sta_gateway: z.string().ip({ version: 'v4' }).optional(),
    sta_subnet: z.string().ip({ version: 'v4' }).optional(),
    sta_primary_dns: z.string().ip({ version: 'v4' }).optional(),
    sta_secondary_dns: z.string().ip({ version: 'v4' }).optional(),
    ap_ssid: z.string().optional(),
    ap_password: z.string().optional(),
    ap_config_ip: z.string().ip({ version: 'v4' }).optional(),
    ap_config_gateway: z.string().ip({ version: 'v4' }).optional(),
    ap_config_subnet: z.string().ip({ version: 'v4' }).optional(),
    hostname: z.string().optional(),
});
//# sourceMappingURL=network-schema.js.map