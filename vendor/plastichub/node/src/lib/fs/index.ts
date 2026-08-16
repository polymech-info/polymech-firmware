const fg = require('fast-glob');

import * as path from 'path';
import * as bluebird from 'bluebird';
import { Converter } from 'showdown';
import { sync as read } from '@xblox/fs/read';
import { sync as exists } from '@xblox/fs/exists';
import { html_beautify } from 'js-beautify';

export { sync as read } from '@xblox/fs/read';
export { sync as exists } from '@xblox/fs/exists';
export { sync as dir } from '@xblox/fs/dir';
export { sync as write } from '@xblox/fs/write';

import { sync as write } from '@xblox/fs/write';

import { Helper } from '../process/index';
import { firstOf, lastOf } from '../common/array';
import { img } from '../content/html';

const IMAGES_GLOB = '*.+(JPG|jpg|png|PNG|gif)';

export const files = (dir, glob) => fg.sync(glob, { dot: true, cwd: dir, absolute: true }) as [];
export const images = (source) => files(source, IMAGES_GLOB) as any[];
export const head_image = (_images) => firstOf(_images);
export const tail_image = (_images) => lastOf(_images);

export async function resize_images(files) {
    return bluebird.mapSeries(files, (file: string) => {
        const inParts = path.parse(file);
        const promise = Helper.run(inParts.dir, 'convert',
            [
                `"${inParts.base}"`,
                '-quality 70',
                '-resize 1980',
                '-sharpen 0x1.0',
                `"${inParts.name}${inParts.ext}"`
            ]);
        return promise;
    });
}

export const md2html = (content) => {
    let converter = new Converter({ tables: true });
    converter.setOption('literalMidWordUnderscores', 'true');
    return converter.makeHtml(content);
}


export const toHTML = (path, markdown) => {
    const content = read(path, 'string') as string;
    if (!markdown) {
        let converter = new Converter({ tables: true });
        converter.setOption('literalMidWordUnderscores', 'true');
        return converter.makeHtml(content);
    } else {
        return content;
    }
}

const jekyllNop = "---\n#jekyll\n---\n";
const frontMatter = /^---[.\r\n]*---/;

export const thumbs = (source: string, meta: boolean = true, sep: string = "<hr/>") => {
    let pictures = images(source);
    let content = "";
    pictures.forEach((f, i) => {
        if (meta) {
            let picMD = path.resolve(path.join(path.parse(f).dir, path.sep, path.parse(f).name + '.md'));
            if (exists(picMD)) {
                const picMDContent = read(picMD, "string") as string;
                if (picMDContent.length > 3 && picMDContent !== jekyllNop) {
                    content += picMDContent.substr(picMDContent.lastIndexOf('---') + 3, picMDContent.length)
                    content += "\n";
                } else {
                    write(picMD, jekyllNop);
                }
            } else {
                write(picMD, jekyllNop);
            }
        }

        content += img(`./${path.parse(f).base}`, `${i + 1}. `, path.parse(f).base);
        content += "\n";
        content += sep;
    });
    return html_beautify(content);
}