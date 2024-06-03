
const information = document.getElementById('info')
information.innerText = `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`


const caccC1_min = document.getElementById("parameter1Min")
const caccC1_max = document.getElementById("parameter1Max")
const caccC1_cnf = document.getElementById("parameter1Config")

const caccOmega_min = document.getElementById("parameter2Min")
const caccOmega_max = document.getElementById("parameter2Max")
const caccOmega_cnf = document.getElementById("parameter2Config")
const caccOmega_uni = document.getElementById("parameter2Unit")

const caccXi_min = document.getElementById("parameter3Min")
const caccXi_max = document.getElementById("parameter3Max")
const caccXi_cnf = document.getElementById("parameter3Config")


var errorDialog = new bootstrap.Modal(document.getElementById('errorDialog'), {backdrop: 'static'})
const dialogText = document.getElementById('errorDialogText')



document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
});

document.getElementById('closeBtn').addEventListener('click', () => {
    window.utils.closeCall();
});

document.getElementById("okayBtn").addEventListener('click',()=>{
    const data =[
        {
            min: parseFloat(caccC1_min.value),
            max: parseFloat(caccC1_max.value),
            config: caccC1_cnf.value
        },
        {
            min: parseFloat(caccOmega_min.value),
            max: parseFloat(caccOmega_max.value),
            config: caccOmega_cnf.value,
            unit: caccOmega_uni.value
        },
        {
            min: parseFloat(caccXi_min.value),
            max: parseFloat(caccXi_max.value),
            config: caccXi_cnf.value
        }
    ];

    for (const param of data) {
        if (param.max <= param.min) {
            dialogText.innerText = `Error: Max value must be greater than Min value for ${param.config}`;
            errorDialog.show();
            return;
        }
    }

    window.utils.sendParameterData(data).then(result =>{
        if (result.success){
            window.utils.goNext();
        }else{
            dialogText.innerText = result.error;
            errorDialog.show()
        }
    });
});

document.addEventListener("DOMContentLoaded", () => {
    window.utils.getProjectData().then(result => {
        if (result.parameterData && Object.keys(result.parameterData).length > 0) {
            const parameter1Data = result.parameterData[0]; 
            caccC1_min.value = parameter1Data.min;
            caccC1_max.value = parameter1Data.max;
            caccC1_cnf.value = parameter1Data.config;

            const parameter2Data = result.parameterData[1]; 
            caccOmega_min.value = parameter2Data.min;
            caccOmega_max.value = parameter2Data.max;
            caccOmega_cnf.value = parameter2Data.config;
            
            const unitOptions = caccOmega_uni.options;
            for (let i = 0; i < unitOptions.length; i++) {
                if (unitOptions[i].value === parameter2Data.unit) {
                    unitOptions[i].selected = true;
                    break;
                }
            }

            const parameter3Data = result.parameterData[2]; 
            caccXi_min.value = parameter3Data.min;
            caccXi_max.value = parameter3Data.max;
            caccXi_cnf.value = parameter3Data.config;
        }
    }).catch(error => {
        console.error('Error');
    });
});


