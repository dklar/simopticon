
const { app, BrowserWindow, ipcMain, dialog, Notification } = require('electron')
const { spawn } = require('child_process');
const fs = require('fs');
const path = require('node:path');
const { resolve } = require('path');
const JSZip = require('jszip');

let childProcess;
let mainWindow;
let programSettings;
let projectConfiguration = {
  name: "",
  isNewProject: true,
  projectDir: "",
  mainfilePath: "",
  parameterPath: "",
  optimizerPath: "",
  runnerPath: "",
  evalPath: "",
  mainFileData: {},
  parameterData: {},
  optimizerData: {},
  runnerData: {},
  evalData: {}
}

const stateMachine = {
  currentState: "index.html",
  previousStates: [],
  transitions: {
    "index.html": { transitions: ["NewProject.html", "settings.html", "results.html","resultmanagment.html"], backAllowed: false },
    "settings.html": { transitions: ["index.html"], backAllowed: true },
    "resultmanagment.html":{transitions: ["index.html"], backAllowed: true},
    "NewProject.html": { transitions: ["ParameterOptimizerEdit.html", "index.html","running.html"], backAllowed: true },
    "ParameterOptimizerEdit.html": { transitions: ["ParameterRunnerEdit.html", "NewProject.html"], backAllowed: true },

    "ParameterRunnerEdit.html": { transitions: ["ParameterOptimizerEdit.html", "ParameterParameterEdit.html"], backAllowed: true },
    "ParameterParameterEdit.html": { transitions: ["ParameterRunnerEdit.html", "ParameterEvaluationEdit.html"], backAllowed: true },
    "ParameterEvaluationEdit.html": { transitions: ["ParameterParameterEdit.html", "ParameterSimopticonEdit.html"], backAllowed: true },
    "ParameterSimopticonEdit.html": { transitions: ["ParameterEvaluationEdit.html", "running.html"], backAllowed: true },

    "running.html": { transitions: ["results.html","index.html"], backAllowed: false },
    "results.html": { transitions: ["index.html"], backAllowed: false },
  },
  transitionTo(newState) {
    const possibleTransitions = this.transitions[this.currentState].transitions;
    if (possibleTransitions.includes(newState)) {
      this.previousStates.push(this.currentState);
      this.currentState = newState;
      if (newState === "index.html"){
        this.clearStack();
      }
      console.log(`Transition from ${this.previousStates[this.previousStates.length - 1]} to ${this.currentState}`);
      return true;
    } else {
      console.error(`Transition from ${this.currentState} to ${newState} not possible`);
    }
    return false;
  },
  back() {
    if (this.previousStates.length > 0) {
      if (this.transitions[this.currentState].backAllowed) {
        this.currentState = this.previousStates.pop();
        console.log(`Transition back to ${this.currentState}`);
        return true;
      } else {
        console.error(`Back not allowed from state ${this.currentState}`);
        return false;
      }
    } else {
      console.error("No previous state available");
      return false;
    }
  },
  next() {
    const possibleTransitions = this.transitions[this.currentState].transitions;
    if (possibleTransitions.length === 2 && possibleTransitions.includes(this.previousStates[this.previousStates.length - 1])) {
      const nextState = possibleTransitions.find(state => state !== this.previousStates[this.previousStates.length - 1]);
      if (nextState) {
        this.previousStates.push(this.currentState);
        this.currentState = nextState;
        console.log(`Transition from ${this.previousStates} to ${this.currentState}`);
        return true;
      }
    } else {
      console.error(`Next state not available from ${this.currentState}`);
    }
    return false;
  },
  clearStack(){
    previousStates = []
  }
};

// Handle creating/removing shortcuts on Windows when installing/uninstalling.
if (require('electron-squirrel-startup')) {
  app.quit();
}


function resolvePath(basePath, relativePath) {
  return path.isAbsolute(relativePath) ? relativePath : path.join(basePath, relativePath);
}

function removeComments(jsonString) {
  //removes comments starting with "//". because it's invalid for offical json objects
  return jsonString.replace(/\/\/.*?\n/g, '');
}


function handleGoNext() {
  if (stateMachine.next()) {
    mainWindow.loadFile(stateMachine.currentState);
  }
}

function handleGoBack() {
  if (stateMachine.back()) {
    mainWindow.loadFile(stateMachine.currentState);
  }
}

function handleGoState(event, state) {
  if (stateMachine.transitionTo(state)) {
    mainWindow.loadFile(stateMachine.currentState);
  }
}

function generateFormFromJSON(jsonData) {
  let formHTML = '<form id="jsonForm" class="mb-3 row">';
  formHTML += '<fieldset>'

  // Iteriere über jedes Objekt im JSON-Array
  jsonData.forEach((obj, index) => {
    formHTML += '<div class="card mb-4">';

    const title = obj.hasOwnProperty('ui_titel') ? obj.ui_titel : `Parameter ${index + 1}`;
    formHTML += `<div class="card-header">${title}</div>`;
    formHTML += `<div class="card-body">`;
    Object.keys(obj).forEach(key => {
      if (key === 'ui_titel') return;
      formHTML += `<div class="form-floating mb-3">`;
      if (typeof obj[key] === 'number') {
        formHTML += `<input type="number" class="form-control" id="${key}_${index}" placeholder="${obj[key]}">`;
      } else {
        formHTML += `<input type="text" class="form-control" id="${key}_${index}" placeholder="${obj[key]}">`;
      }
      formHTML += `<label for="${key}_${index}" class="form-label">${key}</label>`;
      formHTML += `</div>`;
    });

    formHTML += '</div></div>';
  });
  formHTML += `<button type="button" class="btn btn-primary mb-3 float-end" id="okayBtn">Continue</button>`
  formHTML += '</fieldset>';
  formHTML += '</form>';

  return formHTML;
}

function writeProjectToDisk() {
  try {
    if (projectConfiguration.isNewProject){
      fs.mkdirSync(projectConfiguration.projectDir);
    }
    fs.writeFileSync(projectConfiguration.mainfilePath, JSON.stringify(projectConfiguration.mainFileData));
    fs.writeFileSync(projectConfiguration.parameterPath, JSON.stringify(projectConfiguration.parameterData));
    fs.writeFileSync(projectConfiguration.optimizerPath, JSON.stringify(projectConfiguration.optimizerData));
    fs.writeFileSync(projectConfiguration.runnerPath, JSON.stringify(projectConfiguration.runnerData));
    fs.writeFileSync(projectConfiguration.evalPath, JSON.stringify(projectConfiguration.evalData));
    return true;
  } catch (error) {
    console.error("Error writing project data to disk:", error);
    return false;
  }
}

function checkValidSimopticon(jsonData) {
  if (
    !jsonData ||
    typeof jsonData !== "object" ||
    !jsonData.hasOwnProperty("controller") ||
    !jsonData.hasOwnProperty("optimizer") ||
    !jsonData.hasOwnProperty("runner") ||
    !jsonData.hasOwnProperty("evaluation") ||
    typeof jsonData.controller !== "object" ||
    typeof jsonData.optimizer !== "object" ||
    typeof jsonData.runner !== "object" ||
    typeof jsonData.evaluation !== "object" ||
    typeof jsonData.controller.updateInterval !== "number" ||
    typeof jsonData.controller.nrTopEntries !== "number" ||
    typeof jsonData.controller.keepResultFiles !== "boolean" ||
    typeof jsonData.controller.params !== "string" ||
    typeof jsonData.optimizer.optimizer !== "string" ||
    typeof jsonData.optimizer.config !== "string" ||
    typeof jsonData.runner.runner !== "string" ||
    typeof jsonData.runner.config !== "string" ||
    typeof jsonData.evaluation.evaluation !== "string" ||
    typeof jsonData.evaluation.config !== "string"
  ) {
    return false;
  }

  return true;
}

function checkValidParameterConfig(data) {
  if (!Array.isArray(data)) {
    return false;
  }
  for (let i = 0; i < data.length; i++) {
    const configItem = data[i];
    if (!configItem.hasOwnProperty('min') || !configItem.hasOwnProperty('max') || !configItem.hasOwnProperty('config')) {
      return false;
    }
    if (typeof configItem.min !== 'number' || typeof configItem.max !== 'number') {
      return false;
    }
    if (typeof configItem.config !== 'string') {
      return false;
    }
    if (configItem.hasOwnProperty('unit') && typeof configItem.unit !== 'string') {
      return false;
    }
  }

  // Rückgabe true, wenn alle Überprüfungen bestanden wurden
  return true;
}

function checkValidOptimizerConfig(obj) {
  if (typeof obj !== 'object' || obj === null) {
    return false;
  }

  if (!obj.hasOwnProperty('output') || !obj.hasOwnProperty('stopCon')) {
    return false;
  }
  if (typeof obj.output !== 'object' || obj.output === null ||
    !obj.output.hasOwnProperty('progress') || !obj.output.hasOwnProperty('values')) {
    return false;
  }
  if (typeof obj.stopCon !== 'object' || obj.stopCon === null ||
    !obj.stopCon.hasOwnProperty('evaluations') || !obj.stopCon.hasOwnProperty('hyrects') ||
    !obj.stopCon.hasOwnProperty('minutes') || !obj.stopCon.hasOwnProperty('accuracy')) {
    return false;
  }
  const stopCon = obj.stopCon;
  const conditions = ['evaluations', 'hyrects', 'minutes', 'accuracy'];
  for (const condition of conditions) {
    if (typeof stopCon[condition] !== 'object' || stopCon[condition] === null ||
      !stopCon[condition].hasOwnProperty('useCondition') ||
      !stopCon[condition].hasOwnProperty('n')) {
      return false;
    }
  }
  return true;
}

function checkValidRunnerConfig(data) {
  if (typeof data !== 'object') {
    return false;
  }

  // Überprüfen, ob alle erforderlichen Schlüssel vorhanden sind
  if (!data.hasOwnProperty('controller') || !data.hasOwnProperty('nrThreads') || !data.hasOwnProperty('repeat') || !data.hasOwnProperty('scenarios') || !data.hasOwnProperty('configDirectory')) {
    return false;
  }

  // Überprüfen des 'controller'-Objekts
  if (typeof data.controller !== 'object' ||
    typeof data.controller.controller !== 'string' ||
    typeof data.controller.headway !== 'number' ||
    typeof data.controller.insertDistance !== 'number' ||
    typeof data.controller.insertHeadway !== 'number') {
    return false;
  }

  // Überprüfen des 'nrThreads'-Werts
  if (typeof data.nrThreads !== 'number') {
    return false;
  }

  // Überprüfen des 'repeat'-Werts
  if (typeof data.repeat !== 'number') {
    return false;
  }

  // Überprüfen des 'scenarios'-Arrays
  if (!Array.isArray(data.scenarios) || data.scenarios.length === 0 || !data.scenarios.every(scenario => typeof scenario === 'string')) {
    return false;
  }

  // Überprüfen des 'configDirectory'-Werts
  if (typeof data.configDirectory !== 'string') {
    return false;
  }

  return true;
}

function checkValidEvaluationConfig(data) {
  if (typeof data !== 'object') {
    return false;
  }
  if (!data.hasOwnProperty('nrThreads') || !data.hasOwnProperty('pythonScript') || !data.hasOwnProperty('omnetppDirectory')) {
    return false;
  }
  if (typeof data.nrThreads !== 'number' || typeof data.pythonScript !== 'string' || typeof data.omnetppDirectory !== 'string') {
    return false;
  }
  return true;
}

function openProject(fileNamePath){
  try {
    const fileContent = fs.readFileSync(fileNamePath, 'utf8');
    const mainFileData = JSON.parse(removeComments(fileContent));
    if (checkValidSimopticon(mainFileData)) {
      console.log("valid document");
      const mainDir = path.dirname(fileNamePath);

      const parameterConfigPath = resolvePath(mainDir, mainFileData.controller.params);
      const optimizerConfigPath = resolvePath(mainDir, mainFileData.optimizer.config);
      const runnerConfigPath = resolvePath(mainDir, mainFileData.runner.config);
      const evalConfigPath = resolvePath(mainDir, mainFileData.evaluation.config);

      const parameterData = fs.readFileSync(parameterConfigPath, 'utf8');
      const optimizerData = fs.readFileSync(optimizerConfigPath, 'utf8');
      const runnerData = fs.readFileSync(runnerConfigPath, 'utf8');
      const evalData = fs.readFileSync(evalConfigPath, 'utf8');
  
      const parameterDataObj = JSON.parse(removeComments(parameterData));
      const optimizerDataObj = JSON.parse(removeComments(optimizerData));
      const runnerDataObj = JSON.parse(removeComments(runnerData));
      const evalDataObj = JSON.parse(removeComments(evalData));
  
      if (!checkValidEvaluationConfig(evalDataObj)){
        console.error("evalData err");
        return false;
      }
      if (!checkValidRunnerConfig(runnerDataObj)){
        console.error("runnerData err");
        return false;
      }
      if (!checkValidOptimizerConfig(optimizerDataObj)){
        console.error("optimizerDataObj err");
        return false;
      }
      if (!checkValidParameterConfig(parameterDataObj)){
        console.error("parameterDataObj err");
        return false;
      }
      
      projectConfiguration.isNewProject = false;
      projectConfiguration.projectDir = mainDir;
      projectConfiguration.name = path.basename(fileNamePath).split('.')[0];
      projectConfiguration.parameterData = parameterDataObj;
      projectConfiguration.optimizerData = optimizerDataObj;
      projectConfiguration.runnerData = runnerDataObj;
      projectConfiguration.evalData = evalDataObj;
      projectConfiguration.mainFileData = mainFileData;
  
      projectConfiguration.parameterPath = parameterConfigPath;
      projectConfiguration.optimizerPath = optimizerConfigPath;
      projectConfiguration.runnerPath = runnerConfigPath;
      projectConfiguration.evalPath = evalConfigPath;
      projectConfiguration.mainfilePath = fileNamePath;
      console.log("Project okay ");
      return true;
    } else {
      console.error('Error no Simopticon file');
      return false;
    }
  } catch (error) {
    console.error('Error while opening project:', error);
    return false;
  }
}

async function openfileDialogProcedure() {
  console.log("You should see " + programSettings.projectPath);
  const { canceled, filePaths } = await dialog.showOpenDialog(mainWindow, {
    defaultPath: programSettings.projectPath,
    filters: [{ name: 'JSON', extensions: ['json'] }],
    properties: ['openFile'],
    title: 'Datei auswählen'
  });
  if (!canceled) {
    console.log("You selected one file"+ filePaths[0]);
    const selectedFilePath = filePaths[0];
    if (openProject(selectedFilePath)) {
      projectName = projectConfiguration.name
      return { success: true, msg: '', projectName: projectName };
    } else {
      return { success: false, msg: "Failed to open the JSON project file.\nThis file seems not to be a simopticon file", projectName: "" }
    }
  } else {
    return { success: false, msg: "To create a new project, enter name!", projectName: "" }
  }
}

async function createProject(event, newProjectName) {
  const projectDir = path.join(programSettings.projectPath, newProjectName);
  if (fs.existsSync(projectDir)) {
    projectConfiguration.isNewProject = false;
    return false;
  } else {
    projectConfiguration.name = newProjectName;
    projectConfiguration.projectDir = projectDir;
    projectConfiguration.isNewProject = true;
    return true;
  }
}

async function getRunnerData(event, runnerData) {
  if (checkValidRunnerConfig(runnerData)) {
    const tmp = path.join(projectConfiguration.projectDir, projectConfiguration.name + "_runner.json");
    projectConfiguration.runnerData = runnerData;
    projectConfiguration.runnerPath = tmp;
    return {
      success: true,
      message: 'Runner data has been updated in project configuration'
    };
  } else {
    return {
      success: false,
      message: "Invalid controller configuration."
    };
  }
}

async function getParameterData(event, data) {
  if (checkValidParameterConfig(data)) {
    const tmp = path.join(projectConfiguration.projectDir, projectConfiguration.name + "_parameter.json");
    projectConfiguration.parameterData = data;
    projectConfiguration.parameterPath = tmp;
    return {
      success: true,
      message: 'Parameter data has been updated in project configuration'
    };
  } else {
    return {
      success: false,
      message: "Invalid parameter configuration."
    };
  }
}

async function getOptimizerData(event, optimizerData) {
  if (checkValidOptimizerConfig(optimizerData)) {
    const tmp = path.join(projectConfiguration.projectDir, projectConfiguration.name + "_optimizer.json");

    projectConfiguration.optimizerData = optimizerData;
    projectConfiguration.optimizerPath = tmp;

    return {
      success: true,
      message: 'Optimizer data has been updated in project configuration'
    };
  } else {
    return {
      success: false,
      message: "Invalid optimizer configuration."
    };
  }
}

async function getEvaluationData(event, evaluationData) {
  if (checkValidEvaluationConfig(evaluationData)) {
    const tmp = path.join(projectConfiguration.projectDir, projectConfiguration.name + "_evaluation.json");
    projectConfiguration.evalData = evaluationData;
    projectConfiguration.evalPath = tmp;
    return {
      success: true,
      message: 'Evaluation data has been updated in project configuration'
    };
  } else {
    return {
      success: false,
      message: "Invalid evaluation configuration."
    };
  }
}

async function getMainfileData(event, data) {
  const tmp = path.join(projectConfiguration.projectDir, projectConfiguration.name + "_simopticon.json");
  projectConfiguration.mainfilePath = tmp;
  projectConfiguration.mainFileData = data;
  if (writeProjectToDisk()) {
    return {
      success: true,
      message: 'Project configuration written to file',
      path: tmp
    };
  } else {
    return {
      success: false,
      message: "Error writing project data to disk"
    };
  }
}



async function saveZipFile(event, data, folderName, zipFileName) {
  try {
      const zip = await JSZip.loadAsync(data);
      const projectDir = projectConfiguration.projectDir;
      const folderPath = `${projectDir}/${folderName}`;
      const plotFolderPath = `${folderPath}/plot`;
      
      if (fs.existsSync(folderPath)) {
          fs.rmSync(folderPath, { recursive: true });
      }
      fs.mkdirSync(folderPath, { recursive: true });
      fs.mkdirSync(plotFolderPath);
      for (const [relativePath, zipEntry] of Object.entries(zip.files)) {
          if (!zipEntry.dir) {
              const fileData = await zipEntry.async('nodebuffer');
              
              if (zipEntry.name.endsWith('.png')){
                fs.writeFileSync(`${plotFolderPath}/${zipEntry.name}`, fileData);
              }else{
                fs.writeFileSync(`${folderPath}/${zipEntry.name}`, fileData);
              }
          }
      }
      fs.writeFileSync(`${folderPath}/${zipFileName}`, data);

      return { success: true, projectPath: projectDir };
  } catch (error) {
      console.error('Error by saving ZIP file: ', error);
      return { success: false, message: 'Error by saving ZIP file' };
  }
}

async function getImages(event, imageFolderPath){
  try {
    const files = fs.readdirSync(imageFolderPath).filter(file => file.endsWith('.png'));
    const imagePaths = files.map(file => path.join(imageFolderPath, file));
    return { success: true, imagePaths };
  } catch (error) {
      console.error('Error by loading images in main:', error);
      return { success: false, error: error.message };
  }
}

async function getSimulations(event, folderPath) {
  try {
      const subdirectories = fs.readdirSync(folderPath)
          .filter(entry => fs.statSync(path.join(folderPath, entry)).isDirectory());
      return { success: true, simulations: subdirectories };
  } catch (error) {
      console.error('Error by loading simulation: ', error);
      return { success: false, error: error.message };
  }
}


async function getprojectData(event) {
  return projectConfiguration;
}

ipcMain.handle('openFile', (event, arg) => {
  console.log("filedialog")
  dialog.showOpenDialog(mainWindow, {
    defaultPath: path.join(app.getPath('home'), "Uni/simopticonLib/config/"),
    filters: [{ name: 'JSON', extensions: ['json'] }],
    properties: ['openFile'],
    title: 'Datei auswählen'
  }).then(result => {
    if (!result.canceled) {
      const selectedFilePath = result.filePaths[0];
      try {
        console.log('File: ', selectedFilePath);
        const data = fs.readFileSync(selectedFilePath, 'utf8');
        const jsonContent = JSON.parse(data);
        console.log(jsonContent);
        resolve(jsonContent);
      } catch (error) {
        console.error('Error when reading the file or parsing the JSON:', error);
        return {};
      }
    }
  }).catch(err => {
    console.error('Error when opening the file selection dialog:', err);
    return {}
  });
});

ipcMain.handle('listProjects', (event) => {
  try {
    const directoryPath = programSettings.projectPath;
    const items = fs.readdirSync(directoryPath).filter(item => {
      const itemPath = path.join(directoryPath, item);
      return fs.statSync(itemPath).isDirectory() && !item.startsWith('.');
    });

    const projects = items.map(item => {
      const itemPath = path.join(directoryPath, item);
      const stats = fs.statSync(itemPath);
      return {
        name: item,
        modified: stats.mtime,
        path: itemPath
      };
    });

    return projects;
  } catch (error) {
    return { error: error.message };
  }
});

ipcMain.on('getSettings', (event) => {
  const userDataPath = app.getPath('userData');
  const programSettingsPath = path.join(userDataPath, 'settings.json');
  try {
    const programSettings = JSON.parse(fs.readFileSync(programSettingsPath, 'utf8'));
    event.returnValue = programSettings;
  } catch (err) {
    console.error('Error reading program settings:', err);
    event.returnValue = null;
  }
});

ipcMain.on("saveSettings", (event, data) => {
  console.log("Save Settings")
  console.log(data);
  const userDataPath = app.getPath('userData');
  const windowStatePath = path.join(userDataPath, 'settings.json');
  try {
    fs.writeFileSync(windowStatePath, JSON.stringify(data));
  } catch (err) { 
    console.error('Error reading program settings:', err);
  }
});

ipcMain.handle('createProject', createProject);

ipcMain.handle('getRunnerData', getRunnerData);
ipcMain.handle('getParameterData', getParameterData);
ipcMain.handle('getOptimizerData', getOptimizerData);
ipcMain.handle('getEvaluationData', getEvaluationData);
ipcMain.handle('getMainfileData', getMainfileData);


ipcMain.handle('openProject',openProject)

ipcMain.handle('getProjectData', getprojectData);
ipcMain.handle('dialog:openFile2', openfileDialogProcedure);

ipcMain.handle('goNext', handleGoNext);
ipcMain.handle('goBack', handleGoBack);
ipcMain.handle('goToState', handleGoState);

ipcMain.handle('saveZipFile',saveZipFile);

ipcMain.handle('getImages',getImages);
ipcMain.handle('getSimulations',getSimulations);


function loadSettings(filepath) {
  let result = {};
  if (fs.existsSync(filepath)) {
    result.firstRun = false;
    result.programSettings = JSON.parse(fs.readFileSync(filepath, 'utf8'));
  } else {
    projectsPath = path.join(app.getPath('home'),'PlexeProjects');
    result.firstRun = true;
    result.programSettings = {
      sumoPath: "",
      plexePath: "",
      startServerLocal: false,
      serverExecutable:"/home/dklar/Uni/RestServer/build/CrowRestServerSimopticon",
      serverPath: "127.0.0.1",
      port: 8080,
      projectPath: projectsPath
    };
    console.log(result.programSettings);
    fs.writeFileSync(filepath, JSON.stringify(result.programSettings));
  }
  return result;
}
const createWindow = () => {
  // Create the browser window.
  mainWindow = new BrowserWindow({
    width: 800,
    height: 600,
    minHeight: 600,
    minWidth: 800,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
    }
  });

  try {
    const windowStatePath = path.join(app.getPath('userData'), 'windowState.json');
    const windowState = JSON.parse(fs.readFileSync(windowStatePath, 'utf8'));
    if (windowState.max) {
      mainWindow.maximize();
    } else {
      if (windowState.width && windowState.height) {
        mainWindow.setSize(windowState.width, windowState.height);
      }
      if (windowState.x && windowState.y) {
        mainWindow.setPosition(windowState.x, windowState.y);
      }
    }
  } catch (err) { }

  mainWindow.loadFile(stateMachine.currentState);
  mainWindow.removeMenu();
  mainWindow.webContents.openDevTools();

  mainWindow.on('close', () => {
    const windowState = {
      max: mainWindow.isMaximized(),
      width: mainWindow.getBounds().width,
      height: mainWindow.getBounds().height,
      x: mainWindow.getBounds().x,
      y: mainWindow.getBounds().y
    };
    fs.writeFileSync(path.join(app.getPath('userData'), 'windowState.json'), JSON.stringify(windowState));
  });

};

function startServer(serverPath){
  const args = [];
  const options = {
    env: process.env
  };
  childProcess = spawn(serverPath, args, options);

  childProcess.stdout.on('data', (data) => {
    console.log(`stdout: ${data}`);
  });

  childProcess.stderr.on('error', (data) => {
    console.error(`stderr: ${data}`);
    dialog.showErrorBox('Warning', `${data}`);
  });

  childProcess.on('close', (code) => {
    console.log(`child process exited with code ${code}`);
  });

}


app.whenReady().then(() => {
  createWindow();

  const res = loadSettings(path.join(app.getPath('userData'), 'settings.json'));
  console.log("Userdata is stored at " + path.join(app.getPath('userData'), 'settings.json'));
  programSettings = res.programSettings;
  if (res.firstRun){
    firstRunWindow = new BrowserWindow({
      width: 400,
      height: 200,
      minHeight: 200,
      minWidth: 400,
      maxHeight:200,
      maxWidth:400,
      maximizable:false,
      minimizable:false,
      parent: mainWindow,
      modal:true,
      show: false,
      alwaysOnTop:true,
      title:"First run information"
    });
    firstRunWindow.loadFile('welcomeInformation.html');
    firstRunWindow.show();
  }

  //let programPath = '/home/dklar/Uni/RestServer/build/CrowRestServerSimopticon';
  if (programSettings.serverPath === ""){
    const notification = new Notification({
      title: 'Welcome',
      body: 'No adress available',
      hasReply: false
    });
    notification.show()
  }else{
    if (programSettings.startServerLocal){
      startServer(programSettings.serverPath)
      console.log("Server started local");
    }else{
      console.log("Remote server");
    }
  }
  
  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
});

app.on('ready', () => {
});

app.on('before-quit', () => {
  if (childProcess) {
    childProcess.kill();
  }
});

app.on('quit', () => {

})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit()
})