// Modules to control application life and create native browser window
const electron = require('electron')
const { app, BrowserWindow } = require('electron')
const path = require('path')
const { register, dock, unregister } = require('./electron-application-desktop-toolbar')
const ipc = electron.ipcMain

function createWindow () {
  // Create the browser window.
    const mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        //x: 3540,
        //y: 0,
        frame: false,
        titleBarStyle: 'none',
        alwaysOnTop: true,
        type: 'toolbar',
        backgroundColor: '#283243',
        resizable: false,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            nodeIntegration: true,
            contextIsolation: false,
        }
    })

    mainWindow.setAlwaysOnTop(true, 'screen')
    mainWindow.setVisibleOnAllWorkspaces(true);
    mainWindow.setMenuBarVisibility(true)
    mainWindow.setResizable(false)
    mainWindow.setSkipTaskbar(false)

    winhndle = mainWindow.getNativeWindowHandle();

  // and load the index.html of the app.
    mainWindow.loadFile('index.html')

    callbackfn = function (evt, info) {
        setTimeout(function () {
            mainWindow.setSize(info.width, info.height, false);
            mainWindow.setPosition(info.left, info.top, false);
            console.log("[%s] Docking: %s, {%d, %d, %d, %d}", evt, info.side, info.left, info.top, info.width, info.height);
        }, 1000);
    }

    register(winhndle, callbackfn);

    // Parameters match the output (above)
    // dock(handle, side, left, top, width, height)

    // This example docks to my secondary screen which is to the left
    dock(winhndle, true, -1920, 0, 1920, 100);

    // This example docks to my primary screen
    // dock(winhndle, true, 0, 0, 1920, 100);

    // Open the DevTools.
     //mainWindow.webContents.openDevTools()
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow()

  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', function () {
  unregister(winhndle)
  if (process.platform !== 'darwin') app.quit()
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
ipc.on('close', _ => {
    console.log('closing')
    unregister(winhndle)
    app.quit()
})

ipc.on('move', _ => {
    console.log('moving')
    dock(winhndle, true, 0, 0, 1920, 100)
})