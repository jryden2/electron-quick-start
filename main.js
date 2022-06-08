// Modules to control application life and create native browser window
const {app, BrowserWindow} = require('electron')
const path = require('path')
const { register, dock, unregister } = require('./electron-application-desktop-toolbar')

function createWindow () {
  // Create the browser window.
    const mainWindow = new BrowserWindow({
        width: 600,
        height: 800,
        //x: 3540,
        //y: 0,
        frame: true,
        titleBarStyle: 'none',
        alwaysOnTop: true,
        type: 'toolbar',
        backgroundColor: '#283243',
        resizable: false,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js')
        }
    })

    mainWindow.setAlwaysOnTop(true, 'screen')
    mainWindow.setVisibleOnAllWorkspaces(true);
    mainWindow.setMenuBarVisibility(false)
    mainWindow.setResizable(true)
    mainWindow.setSkipTaskbar(false)

    winhndle = mainWindow.getNativeWindowHandle();

  // and load the index.html of the app.
    mainWindow.loadFile('index.html')

    callbackfn = function (evt, info) {
        setTimeout(function () {
            mainWindow.setSize(info.width, info.height, false);
            mainWindow.setPosition(info.left, info.top, false);
            console.log("Docking: %s, {%d, %d, %d, %d}", info.side, info.left, info.top, info.width, info.height);
        }, 1000);
    }

    register(winhndle, callbackfn);

    dock(winhndle, true, -1920, 0, 0, 100);

    // Open the DevTools.
    // mainWindow.webContents.openDevTools()
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
