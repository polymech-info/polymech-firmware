const fs = require('fs');
const path = require('path');
const cwd = path.resolve('./');
const pkg = JSON.parse(fs.readFileSync('package.json', 'utf8'));
const pkgVersion = pkg.version;
const pkgUrl = pkg.url || '';

async function getGitInfo(workingDir) {
  const simpleGit = require('simple-git');
  const git = simpleGit(workingDir);

  try {
    const [log, status] = await Promise.all([
      git.log(),
      git.status()
    ]);
    return {
      hash: log.latest ? log.latest.hash : 'unknown',
      branch: status.current
    };
  }
  catch (e) {
    console.error('Error fetching git info:', e);
    return { hash: 'unknown', branch: 'unknown' };
  }
}

getGitInfo(cwd).then((info) => {
  const version = `#ifndef VERSION_H
#define VERSION_H
#define VERSION "${pkgVersion}|${info.branch}|${info.hash}"
#define PACKAGE_URL "${pkgUrl}"
#endif`;

  fs.writeFileSync('./src/version.h', version);
  console.log(`wrote ${version} to ./src/version.h`);
});