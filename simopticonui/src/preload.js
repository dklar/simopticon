
// https://www.electronjs.org/docs/latest/tutorial/process-model#preload-scripts
const { contextBridge, ipcRenderer, dialog, os } = require('electron')

window.addEventListener('DOMContentLoaded', () => {
  const replaceText = (selector, text) => {
    const element = document.getElementById(selector)
    if (element) element.innerText = text
  }

  for (const type of ['chrome', 'node', 'electron']) {
    replaceText(`${type}-version`, process.versions[type])
  }
});


contextBridge.exposeInMainWorld('utils', {
  listProjects: () =>ipcRenderer.invoke('listProjects'),

  openFileDialog: () => ipcRenderer.invoke('openFile'),

  openFile2: () => ipcRenderer.invoke('dialog:openFile2'),
  saveSettings: (jsonData) => ipcRenderer.send("saveSettings", jsonData),
  getSettings: () => ipcRenderer.sendSync('getSettings'),

  createProject: (projectName) => ipcRenderer.invoke('createProject', projectName),
  openProject: (data) => ipcRenderer.invoke('openProject',data),

  sendRunnerData: (data) => ipcRenderer.invoke('getRunnerData', data),
  sendParameterData: (data) => ipcRenderer.invoke('getParameterData', data),
  sendOptimizerData: (data) => ipcRenderer.invoke('getOptimizerData', data),
  sendEvaluationData: (data) => ipcRenderer.invoke('getEvaluationData', data),
  sendControllerData: (data) => ipcRenderer.invoke('getControllerData', data),
  sendMainfileData: (data) => ipcRenderer.invoke('getMainfileData', data),
  sendDirectData: (data) => ipcRenderer.invoke('getDirectData',data),

  goBack:() => ipcRenderer.invoke('goBack'),
  goNext: () =>ipcRenderer.invoke('goNext'),
  goToState:(state) => ipcRenderer.invoke('goToState',state),

  saveZipFile:(data,filePath) => ipcRenderer.invoke('saveZipFile',data,filePath),

  getImages: (folderPath) => ipcRenderer.invoke('getImages', folderPath),
  getSimulations: (projectPath) => ipcRenderer.invoke('getSimulations', projectPath),

  getProjectData: () => ipcRenderer.invoke('getProjectData')
});

contextBridge.exposeInMainWorld('versions', {
  node: () => process.versions.node,
  chrome: () => process.versions.chrome,
  electron: () => process.versions.electron
})
