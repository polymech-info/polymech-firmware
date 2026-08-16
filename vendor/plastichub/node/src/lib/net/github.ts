const jsdeliver = 'https://cdn.jsdelivr.net/'

const user = 'plastichub';
const repo = 'products';
const branch = 'master';

const _gh_raw = (path) => `${jsdeliver}/gh/${user}/${repo}@${branch}/${path}`;

export const gh_raw = (path) => _gh_raw(path);

