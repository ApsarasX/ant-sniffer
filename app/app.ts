import * as path from 'path';
import * as url from 'url';
import { app, BrowserWindow, ipcMain } from 'electron';
import * as isDev from 'electron-is-dev';
import { ISniffer, ISnifferInstance } from './sniffer';
const sniffer: ISniffer = require('./sniffer.node');

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
  // 创建浏览器窗口
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
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
  snifferInstance?.closeDev();
});

app.on('will-quit', () => {
  snifferInstance?.closeDev();
});

app.on('activate', function() {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

ipcMain.on('getAllDevices', event => {
  const devs = sniffer.getDevs();
  event.returnValue = devs;
});

let snifferInstance: ISnifferInstance;
let isRunning = false;

ipcMain.on('openDevice', (_, arg) => {
  const devName: string = arg;
  if (!snifferInstance) {
    snifferInstance = sniffer.openDev(devName);
  }
});

ipcMain.on('startCapture', () => {
  if (snifferInstance && !isRunning) {
    snifferInstance.onProgress(onProgress);
    snifferInstance.start();
    isRunning = true;
    setInterval(() => {
      snifferInstance?.keepAlive();
    }, 1000);
  }
});

ipcMain.on('filter-request', (event, value) => {
  const success = snifferInstance.setFilter(value);
  win?.webContents.send('filter-response', success);
});

function onProgress(packet: any) {
  snifferInstance?.keepAlive();
  win?.webContents.send('progress', packet);
}
