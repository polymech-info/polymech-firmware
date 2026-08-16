import { sync } from '@xblox/fs/write';
import { OutputTarget, Options, OutputResult, OutputFormat } from './types';
import { error } from './log';
import { render as format } from './format';

export const stdout = (result: any) => console.log(result);

export const file = (result: string, path: string) => sync(path, result);

export const render = (result: any, options: Options): OutputResult => {
    const report = format(result, options);
    switch (options.target) {
        case OutputTarget.STDOUT: {
            stdout(report);
            return true;
        }
        case OutputTarget.FILE: {
            file(report, options.path);
            return true;
        }
        default: {
            //private, should never happen since options had to be sanitized
            error('output::render Invalid value in options.target');
            return false;
        }
    }
}