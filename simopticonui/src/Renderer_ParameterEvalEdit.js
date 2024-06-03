
const information = document.getElementById('info')
information.innerText = `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`

const omnetppDirectory = document.getElementById('omnetppDirectory');
const nrThreads = document.getElementById('threads');
const pythonScriptPath = document.getElementById('pythonscriptpath');

var errorDialog = new bootstrap.Modal(document.getElementById('errorDialog'), {backdrop: 'static'})
const dialogText = document.getElementById('errorDialogText')


document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
});

document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
});

document.getElementById("okayBtn").addEventListener('click', () => {

    const allInputs = document.querySelectorAll('input[type="text"], input[type="number"]');
    let isValid = true;

    allInputs.forEach(input => {
        input.classList.remove('is-invalid');
        if (!input.value.trim()) {
            input.classList.add('is-invalid');
            isValid = false;
        } else if (input.type === 'number' && parseInt(input.value) < 1) {
            input.classList.add('is-invalid');
            isValid = false;
        }
    });

    if (!isValid) {
        dialogText.innerText = "Please fill in all fields with valid positive numbers.";
        errorDialog.show();
        return;
    }

    const config = {
        "nrThreads": parseInt(nrThreads.value),
        "pythonScript": pythonScriptPath.value,
        "omnetppDirectory": omnetppDirectory.value
    };

    window.utils.sendEvaluationData(config).then(result => {
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
        if (result.evalData && Object.keys(result.evalData).length > 0) {
            omnetppDirectory.value = result.evalData.omnetppDirectory;
            nrThreads.value = result.evalData.nrThreads;
            pythonScriptPath.value = result.evalData.pythonScript
        }
    }).catch(error => {
        console.error('Error');
    });
});