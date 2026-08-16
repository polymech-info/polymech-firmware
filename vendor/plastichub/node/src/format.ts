import { Options, OutputFormat } from "./types";
import { error } from "./log";

export const render = (result: any, options: Options) => {
    switch (options.format) {
        case OutputFormat.text: {
            //@TODO: human readable format
            return JSON.stringify(result, null, 2);
        }
        case OutputFormat.json: {
            return JSON.stringify(result, null, 2);
        }
        default: {
            //private, should never happen since options had to be sanitized
            error('format::render Invalid value in options.format');
            return '';
        }
    }
}