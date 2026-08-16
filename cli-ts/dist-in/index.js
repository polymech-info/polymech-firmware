#!/usr/bin/env node
import yargs from "yargs";
import { hideBin } from "yargs/helpers";
import * as setupNetworkCommand from "./commands/setup-network.js";
import * as setupSettingsCommand from "./commands/setup-settings.js";
import * as setupProfilesCommand from "./commands/setup-profiles.js";
import * as setupSignalsCommand from "./commands/setup-signals.js";
//import * as menuCommand from "./commands/menu.js";
import * as patchConfigCommand from "./commands/patch-config.js";
// import * as buildProtoCommand from "./commands/build-proto.js";
import * as wsClientCommand from "./commands/ws-client.js";
import * as mbCommand from "./commands/mb.js";
import * as patchAppCommand from "./commands/patch-app.js";
import * as dumpCommand from "./commands/dump.js";
import * as restoreCommand from "./commands/restore.js";
import * as testWebCommand from "./commands/test-web.js";
// import * as writeCommand from "./commands/write.js";
yargs(hideBin(process.argv))
    .command(setupNetworkCommand)
    .command(setupSettingsCommand)
    .command(setupProfilesCommand)
    .command(setupSignalsCommand)
    //.command(menuCommand as any)
    .command(patchConfigCommand)
    //.command(buildProtoCommand as any) 
    .command(wsClientCommand)
    .command(mbCommand)
    .command(patchAppCommand)
    .command(dumpCommand)
    .command(restoreCommand)
    .command(testWebCommand)
    // .command(writeCommand as any)
    .demandCommand(1, "You need at least one command before moving on")
    .help()
    .argv;
//# sourceMappingURL=index.js.map