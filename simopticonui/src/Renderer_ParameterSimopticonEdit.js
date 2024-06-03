const information = document.getElementById('info')
information.innerText = `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`

const updateInterval = document.getElementById('updateInterval');
const nrTopEntries = document.getElementById('nrTopEntries');
const keepResultFiles = document.getElementById('keepResultFiles');
const controllerPath = document.getElementById('controllerPath');
const optimizerType = document.getElementById('optimizer')
const optimizerConfigPath = document.getElementById('optimizerConfigPath');
const runnerConfigPath = document.getElementById('runnerConfigPath');
const evalConfigPath = document.getElementById('evalConfigPath');
const evalVal = document.getElementById('eval');
const runner = document.getElementById('runner');



var dialog = new bootstrap.Modal(document.getElementById('Dialog'), {backdrop: 'static'})
const dialogText = document.getElementById('DialogText')

document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
});
document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
});


document.addEventListener('DOMContentLoaded', ()=> {
    window.utils.getProjectData().then(result =>{
        result.name;
        result.pathMainfile;
        controllerPath.value = result.parameterPath;
        optimizerConfigPath.value = result.optimizerPath;
        runnerConfigPath.value = result.runnerPath;
        evalConfigPath.value = result.evalPath;
    });
});

document.getElementById('okayBtn').addEventListener('click', async () => {
    const config = {
        "controller": {
            "updateInterval": parseInt(updateInterval.value),
            "nrTopEntries": parseInt(nrTopEntries.value),
            "keepResultFiles": keepResultFiles.checked,
            "params": controllerPath.value
        },
        "optimizer": {
            "optimizer": optimizerType.value,
            "config": optimizerConfigPath.value
        },
        "runner": {
            "runner": runner.value,
            "config": runnerConfigPath.value
        },
        "evaluation": {
            "evaluation": evalVal.value,
            "config": evalConfigPath.value
        }
    };
    window.utils.sendMainfileData(config).then(result =>{
        if (result.success){
            window.utils.goNext();
        }else{
            dialogText.innerText = result.error;
            dialog.show()
        }
    });
});
