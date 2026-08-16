import * as CLI from 'yargs';
import * as debug from '../../log';
import * as utils from '../../lib/common/strings';
import { read as readSheet } from '../../lib/net/sheets';
import * as path from 'path';
import { sync as read } from '@xblox/fs/read';
import { sync as exists } from '@xblox/fs/exists';
import { sync as dir } from '@xblox/fs/dir';
import { sync as write } from '@xblox/fs/write';
import { Converter } from 'showdown';
import * as SerialPort from 'serialport';

// Programmer for E5DC-QX2ASM-002 Omron

/**
 * 
 * @link Modbus Docs https://www.npmjs.com/package/modbus-serial
 * 
 * @link Serial Port CLI https://serialport.io/docs/guide-cli
 */


const defaultOptions = (yargs: CLI.Argv) => {
    return yargs.option('port', {
        default: 'COM7',
        describe: 'serial port'
    });
};

let options = (yargs: CLI.Argv) => defaultOptions(yargs);

// npm run build ; node ./build/main.js pid:program 
export const register = (cli: CLI.Argv) => {
    return cli.command('pid:program', 'Omron PID programmer', options, async (argv: CLI.Arguments) => {
        if (argv.help) { return; }


        const port = argv.port as string;


        /*
        const sp = new SerialPort(port,{ baudRate: 256000,parity:'even', dataBits:8 });
        parser.on('data', line => console.log(`> ${line}`))*/
        const ModbusRTU = require("modbus-serial");
        const client = new ModbusRTU();


        function end() {
            console.log('read registers');
            client.close();
        }


        function read() {
            client.setID(1);
            // on device number 1.
            console.log('read registers');
            // 01 03 00 00 00 02 C4 0B

            client.readHoldingRegisters(2, 0).then((v
            ) => {
                console.log('v', v);
                end();
            });
        }

        function write() {


            console.log('write registers');
            // 01 03 20 00 00 01 8FCA
            // write the values 0, 0xffff to registers starting at address 0
            // on device number 1.
            // client.writeRegisters(1, [0]).then(read);
        }

        debug.info(`Connecting to ${argv.port}`);
        // open connection to a serial port
        client.connectRTU(argv.port, { baudRate: 9600, parity: 'even' }, read);
    });
};
