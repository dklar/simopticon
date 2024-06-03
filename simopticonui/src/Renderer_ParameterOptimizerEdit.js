const information = document.getElementById('info')
information.innerText = `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`

const maxTriesInput = document.getElementById('maxTries');
const useMaxTriesCheckbox = document.getElementById('useMaxTries');

const accVal_N_Input = document.getElementById('accVal_N');
const accVal_delta_Input = document.getElementById('accVal_delta');
const useAccCheckbox = document.getElementById('useAcc');

const timeValInput = document.getElementById('timeVal');
const useTimeCheckbox = document.getElementById('useTime');

const evalValInput = document.getElementById('evalVal');
const useEvalCheckbox = document.getElementById('useEval');

const useValuesCheckbox = document.getElementById('useValues');
const useProgCheckbox = document.getElementById('useProg');


var errorDialog = new bootstrap.Modal(document.getElementById('errorDialog'), {backdrop: 'static'})
const dialogText = document.getElementById('errorDialogText')


document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
});

document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
})

document.getElementById('okayBtn').addEventListener('click', () => {
    const allInputs = document.querySelectorAll('input[type="text"], input[type="number"]');
    allInputs.forEach(input => {
        input.classList.remove('is-invalid');
    });

    if (parseInt(maxTriesInput.value) < 0 ||
        parseInt(evalValInput.value) < 0 ||
        parseInt(timeValInput.value) < 0 ||
        parseFloat(accVal_delta_Input.value) < 0 ||
        parseInt(accVal_N_Input.value) < 0) {

        if (parseInt(maxTriesInput.value) < 0) {
            maxTriesInput.classList.add('is-invalid');
        }
        if (parseInt(evalValInput.value) < 0) {
            evalValInput.classList.add('is-invalid');
        }
        if (parseInt(timeValInput.value) < 0) {
            timeValInput.classList.add('is-invalid');
        }
        if (parseFloat(accVal_delta_Input.value) < 0) {
            accVal_delta_Input.classList.add('is-invalid');
        }
        if (parseInt(accVal_N_Input.value) < 0) {
            accVal_N_Input.classList.add('is-invalid');
        }
        dialogText.innerText = "Please no negative Numbers";
        errorDialog.show();
        return;
    }
    if ((useMaxTriesCheckbox.checked && !maxTriesInput.value) ||
        (useEvalCheckbox.checked && !evalValInput.value) ||
        (useTimeCheckbox.checked && !timeValInput.value) ||
        (useAccCheckbox.checked && (!accVal_delta_Input.value || !accVal_N_Input.value))) {
        if (useMaxTriesCheckbox.checked && !maxTriesInput.value) {
            maxTriesInput.classList.add('is-invalid');
        }
        if (useEvalCheckbox.checked && !evalValInput.value) {
            evalValInput.classList.add('is-invalid');
        }
        if (useTimeCheckbox.checked && !timeValInput.value) {
            timeValInput.classList.add('is-invalid');
        }
        if (useAccCheckbox.checked && (!accVal_delta_Input.value || !accVal_N_Input.value)) {
            accVal_delta_Input.classList.add('is-invalid');
            accVal_N_Input.classList.add('is-invalid');
        }

        dialogText.innerText = "Please fill all marked fields.";
        errorDialog.show();
        return;
    }

    const config = {
        "output": {
            "progress": useProgCheckbox.checked,
            "values": useValuesCheckbox.checked
        },
        "stopCon": {
            "evaluations": {
                "useCondition": useMaxTriesCheckbox.checked,
                "n": parseInt(maxTriesInput.value)
            },
            "hyrects": {
                "useCondition": useEvalCheckbox.checked,
                "n": parseInt(evalValInput.value)
            },
            "minutes": {
                "useCondition": useTimeCheckbox.checked,
                "n": parseInt(timeValInput.value)
            },
            "accuracy": {
                "useCondition": useAccCheckbox.checked,
                "delta": parseFloat(accVal_delta_Input.value),
                "n": parseInt(accVal_N_Input.value)
            }
        }
    };

    window.utils.sendOptimizerData(config).then(result => {
        if (result.success) {
            window.utils.goNext();
        } else {
            dialogText.innerText = result.error;
            errorDialog.show()
        }
    });

});
    
document.addEventListener("DOMContentLoaded", () => {
    window.utils.getProjectData().then(result => {
        if (result.optimizerData && Object.keys(result.optimizerData).length > 0) {
            maxTriesInput.value = result.optimizerData.stopCon.evaluations.n;
            useMaxTriesCheckbox.checked = result.optimizerData.stopCon.evaluations.useCondition;
            accVal_N_Input.value = result.optimizerData.stopCon.accuracy.n;
            accVal_delta_Input.value = result.optimizerData.stopCon.accuracy.delta;
            useAccCheckbox.checked = result.optimizerData.stopCon.accuracy.useCondition;
            timeValInput.value = result.optimizerData.stopCon.minutes.n;
            useTimeCheckbox.checked = result.optimizerData.stopCon.minutes.useCondition;
            evalValInput.value = result.optimizerData.stopCon.hyrects.n;
            useEvalCheckbox.checked = result.optimizerData.stopCon.hyrects.useCondition;
            useValuesCheckbox.checked = result.optimizerData.output.values;
            useProgCheckbox.checked = result.optimizerData.output.progress;
        }
    }).catch(error => {
        console.error('Error');
    });
});
