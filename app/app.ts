import * as path from 'path';
import * as url from 'url';
import { app, BrowserWindow } from 'electron';
import * as isDev from 'electron-is-dev';

if (isDev) {
  require('electron-debug')({
    isEnabled: true,
    showDevTools: true
  });
}

// 开发模式预览页面
const devUrl = 'http://localhost:3000';

// 生产模式web项目入口
const prodUrl = url.format({
  pathname: path.join(__dirname, '../web/index.html'),
  protocol: 'file',
  slashes: true
});

const appUrl = isDev ? devUrl : prodUrl;

let win: BrowserWindow | null = null;

function createWindow() {
  // 创建浏览器窗口。
  win = new BrowserWindow({
    fullscreen: true,
    webPreferences: {
      nodeIntegration: true
    }
  });
  // 然后加载应用的 index.html。
  win.loadURL(appUrl);
  // 当 window 被关闭，这个事件会被触发。
  win.on('closed', function() {
    win = null;
  });
}

function createDevTools() {
  const {
    default: installExtension,
    REACT_DEVELOPER_TOOLS,
    REDUX_DEVTOOLS
  } = require('electron-devtools-installer');
  // 安装devtron
  const devtronExtension = require('devtron');
  devtronExtension.install();
  // 安装React开发者工具
  installExtension(REACT_DEVELOPER_TOOLS);
  installExtension(REDUX_DEVTOOLS);
}

app.on('ready', function() {
  createWindow();
  if (isDev) {
    createDevTools();
  }
});

// 当全部窗口关闭时退出。
app.on('window-all-closed', function() {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', function() {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
