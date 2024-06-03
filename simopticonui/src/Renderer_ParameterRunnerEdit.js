
const information = document.getElementById('info')
information.innerText = `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`


const controllerNameInput = document.getElementById('controllerName');
const headwayInput = document.getElementById('headway');
const insertDistanceInput = document.getElementById('insertDistance');
const insertHeadwayInput = document.getElementById('insertHeadway');

const nrThreadsInput = document.getElementById('nrThreads');
const repeatInput = document.getElementById('repeat');
const scenariosSelect = document.getElementById('scenarios');
const configDirectoryInput = document.getElementById('configDirectory');

var errorDialog = new bootstrap.Modal(document.getElementById('errorDialog'), {backdrop: 'static'})
const dialogText = document.getElementById('errorDialogText')


document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
});

document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
});

document.getElementById("okayBtn").addEventListener('click',()=>{
    const allInputs = document.querySelectorAll('input[type="text"], input[type="number"], select');
    allInputs.forEach(input => {
        input.classList.remove('is-invalid');
        if (!input.value.trim()) {
            input.classList.add('is-invalid');
        }
    });

    if (document.querySelectorAll('.is-invalid').length > 0) {
        dialogText.innerText = "Please fill in all fields.";
        errorDialog.show();
        return;
    }

    if (parseFloat(headwayInput.value) < 0 || 
        parseFloat(insertDistanceInput.value) < 0 || 
        parseFloat(insertHeadwayInput.value) < 0 || 
        parseInt(nrThreadsInput.value) < 0 || 
        parseInt(repeatInput.value) < 0) {
        if (parseFloat(headwayInput.value) < 0) {
            headwayInput.classList.add('is-invalid');
        }
        if (parseFloat(insertDistanceInput.value) < 0) {
            insertDistanceInput.classList.add('is-invalid');
        }
        if (parseFloat(insertHeadwayInput.value) < 0) {
            insertHeadwayInput.classList.add('is-invalid');
        }
        if (parseInt(nrThreadsInput.value) < 0) {
            nrThreadsInput.classList.add('is-invalid');
        }
        if (parseInt(repeatInput.value) < 0) {
            repeatInput.classList.add('is-invalid');
        }
        dialogText.innerText = "Please enter positive numbers only.";
        errorDialog.show();
        return;
    }

    const plexeRunnerConfig = {
        "controller": {
            "controller": controllerNameInput.value,
            "headway": parseFloat(headwayInput.value),
            "insertDistance": parseFloat(insertDistanceInput.value),
            "insertHeadway": parseFloat(insertHeadwayInput.value)
        },
        "nrThreads": parseInt(nrThreadsInput.value),
        "repeat": parseInt(repeatInput.value),
        "scenarios": Array.from(scenariosSelect.selectedOptions, option => option.value),
        "configDirectory": configDirectoryInput.value
    };

    window.utils.sendRunnerData(plexeRunnerConfig).then(result => {
        if (result.success) {
            window.utils.goNext();
        } else {
            dialogText.innerText = result.error;
            errorDialog.show();
        }
    });

});


document.addEventListener("DOMContentLoaded",()=> {
    window.utils.getProjectData().then(result => {
        if (result.runnerData && Object.keys(result.runnerData).length > 0) {
            controllerNameInput.value = result.runnerData.controller.controller;
            headwayInput.value = result.runnerData.controller.headway;
            insertDistanceInput.value = result.runnerData.controller.insertDistance;
            insertHeadwayInput.value = result.runnerData.controller.insertHeadway;

            nrThreadsInput.value = result.runnerData.nrThreads
            repeatInput.value = result.runnerData.repeat;
            const selectedScenarios = result.runnerData.scenarios;

            if (selectedScenarios && selectedScenarios.length > 0) {
                Array.from(scenariosSelect.options).forEach(option => {
                    if (selectedScenarios.includes(option.value)) {
                        option.selected = true;
                    }
                });
            }
            configDirectoryInput.value = result.runnerData.configDirectory;

        }
    }).catch(error => {
        console.error('Error');
    });
});