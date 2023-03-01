import { app, shell, dialog, ipcMain, BrowserWindow } from 'electron'
import * as path from 'path'
import { electronApp, optimizer } from '@electron-toolkit/utils'
const fs = require('fs')
const request = require('request')
const spawn = require('child_process').spawn;
const kill  = require('tree-kill');

String.prototype.replaceAll = function (search, replacement) {
  return this.replace(new RegExp(search, 'g'), replacement)
}

const processId = {mysql:0,httpd:0}

const Config = require('electron-config')

const gotTheLock = app.requestSingleInstanceLock()

let config = new Config()

let startLog = {}

//get the basepath
const basePath = __dirname.includes('.asar')
  ? path.resolve(__dirname, '../../../..')
  : path.resolve(__dirname, '../..')
const appPath = path.join(basePath, 'app').replace(/\\/g, "/");

startLog['appPath'] = appPath
startLog['basePath'] = basePath

let appConfig
try {
  appConfig = JSON.parse(
    fs.readFileSync(path.join(appPath, 'application.json'), {
      encoding: 'utf8',
      flag: 'r'
    })
  )
} catch (e) {
  dialog.showErrorBox("application.json invalid,`There is a problem with app/application.json`)
  console.log('config error')
  app.exit(0)
}


//const homePage = path.join(__dirname, '../renderer/index.html')

const startPage = `${appPath}/${appConfig.main}`
const baseUrl = `http://localhost:${appConfig.apache_port}${appConfig.baseUrl}`


startLog['baseUrl'] = baseUrl

startLog['appConfig'] = appConfig

let win

if (!gotTheLock) {
  console.log('relaunch app')
  app.exit(0)
}

let opts = {
  autoHideMenuBar: appConfig.menuBar ?? false,
  show: false,
  icon: path.join(__dirname, '../../build/icon.png'),
  webPreferences: {
    preload: path.join(__dirname, '../preload/index.js'),
    nodeIntegration: appConfig.nodeIntegration ?? false,
    sandbox: false
  }
}
Object.assign(opts, config.get('winBounds'))

const loadBaseUrl = () => {
  request(baseUrl, (error, response) => {
    if (!error && response.statusCode == 200) {
      console.log('URL is OK')
      win.loadURL(baseUrl)
    } else {
      console.log('URL is Not available')
      requestBaseUrl()
    }
  })
}

const requestBaseUrl = () => {
  setTimeout(() => {
    loadBaseUrl()
  }, appConfig.loading_delay)
}

const createWindow = () => {
  // Create the browser window.
  win = new BrowserWindow(opts)
  ipcMain.handle('ping', () => {
    return {
      reply: 'pong',
      appPath: appPath,
      basePath: basePath,
      startLog: startLog,
      dir: __dirname
    }
  })

  win.on('ready-to-show', () => {
    win.show()
  })

  win.on('close', () => {
    config.set('winBounds', win.getBounds())
  })

  win.webContents.on('did-fail-load', function () {
      if(appConfig.config.startup) {
        win.loadFile(startPage)
      }
    // REDIRECT TO FIRST WEBPAGE AGAIN
  })

  // Open the DevTools.
  if (appConfig.devtools) {
    win.webContents.openDevTools()
  }

  win.webContents.setWindowOpenHandler((details) => {
    shell.openExternal(details.url)
    return { action: 'deny' }
  })

  // HMR for renderer base on electron-vite cli.
  //win.loadURL(`file://${__dirname}/app/index.html`);

  if(appConfig.config.startup) {
    console.log('Loading startup page');
    win.loadFile(startPage)
  }

  //request Base URL
  if(appConfig.config.apache) {
    console.log('Loading apache url');
    setTimeout(() => {
      requestBaseUrl()
    }, 3000)
  }

}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  // Set app user model id for windows
  electronApp.setAppUserModelId('com.phpbrowserbox.mobile')

  /*
  const server1 = spawn('notepad.exe', [
    'E:/test1.bat' // Path to your file
  ])
  server1.on('error', (err) => {
    console.log('Failed to start bat', err)
  })
  */

  // Default open or close DevTools by F12 in development
  // and ignore CommandOrControl + R in production.
  // see https://github.com/alex8088/electron-toolkit/tree/master/packages/utils
  app.on('browser-window-created', (_, window) => {
    optimizer.watchWindowShortcuts(window)
  })

  createWindow()

  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

//if a second instance is launched
app.on('second-instance', async (_event, _commandLine, _workingDirectory) => {
  console.log('Second Instance')
  win.focus()
})

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    if(processId.httpd) {
       kill(processId.httpd);
    }
    if(processId.mysql) {
       kill(processId.mysql);
    }
    console.log('closing event',processId);
    app.quit()
  }
})

const processConfig = (file: string, path: string, path2?: string) => {
  const destinationFile = appPath + '\\' + path + '\\' + file
  const destinationFile2 = path2 ? appPath + '\\' + path2 + '\\' + file : false
  //const sourceFile = appPath + '\\config\\' + file
  const sourceFile = destinationFile + '.phpbrowserbox'

  try {
  let contents = fs.readFileSync(sourceFile, {
    encoding: 'utf8',
    flag: 'r'
  })

  contents = contents.replaceAll('%phpbrowserbox%', appPath)

  contents = contents.replaceAll('%mysql_port%', appConfig.mysql_port)
  contents = contents.replaceAll('%apache_port%', appConfig.apache_port)

  fs.writeFileSync(destinationFile, contents)

  if(destinationFile2) {
    fs.writeFileSync(destinationFile2, contents)
  }

  } catch(err) {
      dialog.showErrorBox("Config Error",`There is a problem with processing ${sourceFile}`)
    console.log('Error',err);
  }
}

//fix configurations

      if(appConfig.config.apache) {    
          processConfig('php.ini', 'bin\\php', 'bin\\apache\\bin')
          processConfig('httpd.conf', 'bin\\apache\\conf')
      }

      if(appConfig.config.mysql) {    
          processConfig('my.ini', 'bin\\mysql')
      }

//write out start log
fs.writeFileSync(path.join(basePath, 'startup.log'), JSON.stringify(startLog, null, 4))

//executables
const mysqldPath = appPath + '\\bin\\mysql\\bin\\mysqld.exe'
const httpdPath = appPath + '\\bin\\apache\\bin\\httpd.exe'

    if(appConfig.config.mysql) {    
          console.log('starting mysql');
          const mysql = spawn(mysqldPath);
          mysql.on('error', function(err) {
              console.log('Oh noez, teh errurz: ' + err);
        });
        processId.mysql = mysql.pid;
    }

    
    if(appConfig.config.apache) {    
          console.log('starting apache');
          const httpd = spawn(httpdPath);
          httpd.on('error', function(err) {
              console.log('Oh noez, teh errurz: ' + err);
          });
          processId.httpd = httpd.pid;
          console.log(`Started apache ${httpd.pid}`);
    }

  //console.log(startLog);
