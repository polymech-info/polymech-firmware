import { defaults } from './_cli'; defaults();
import * as cli from 'yargs'; 
import { register as registerPIDProgram } from './commands/pid/program'; registerPIDProgram(cli);
var ModbusRTU = require("modbus-serial");

const argv = cli.argv;

if (argv.h || argv.help) {
    cli.showHelp();
    process.exit();
} else if (argv.v || argv.version) {
    // tslint:disable-next-line:no-var-requires

    var client = new ModbusRTU();

    process.exit();
}
