const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');
const projectDir = path.resolve(__dirname, '..');

const buildDir = path.resolve(projectDir, 'build');

const webBuildDir = path.resolve(buildDir, 'web');

if (!fs.existsSync(webBuildDir)) {
  fs.mkdirSync(webBuildDir);
}

const fileList = fs.readdirSync(buildDir);

for (const item of fileList) {
  if (!['web', 'electron'].includes(item)) {
    spawnSync('mv', [path.resolve(buildDir, item), webBuildDir]);
  }
}
