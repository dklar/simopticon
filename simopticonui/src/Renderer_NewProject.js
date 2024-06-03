var errorDialog = new bootstrap.Modal(document.getElementById('exampleModalCenter'), {backdrop: 'static'})
const dialogText = document.getElementById('modalErrorText')

var dialog = new bootstrap.Modal(document.getElementById('modalOkay'), {backdrop: 'static'})
const dialogTextOkay = document.getElementById('modalOkayText')

const projectNameField = document.getElementById('projectName');
const filePathElement = document.getElementById('filePath')

const nextStep = "ParameterOptimizerEdit.html"

document.getElementById('info').innerText =
    `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`


document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
})

document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
})

document.getElementById('startBtn').addEventListener('click',()=>{
    window.utils.goToState("running.html");
})

document.getElementById('editBtn').addEventListener('click',()=>{
    window.utils.goToState(nextStep);
})

document.getElementById('contBtn').addEventListener('click', async () => {
    let errorText;
    if (projectNameField.value===""){
        errorText = "Please enter project name"
    }else{
        const res = await window.utils.createProject(projectNameField.value);
        if (res){
            window.utils.goToState(nextStep);
        }else{
            errorText = "No project could be created"
        }
    }
    if (errorText!=""){
        dialogText.innerText = errorText
        errorDialog.show()
    }
});

document.getElementById('openBtn').addEventListener('click', async () => {
    try {
        const { success, msg, projectName } = await window.utils.openFile2();
        if (success) {
            projectNameField.value = projectName;
            dialogTextOkay.innerText = "The project has been opened\.n Do you want to start it directly or edit it?"
            dialog.show();
        }else{
            dialogText.innerText = msg;
            errorDialog.show();
        }
    } catch (error) {
        dialogText.innerText = error;
        errorDialog.show();
    }
});

document.addEventListener("DOMContentLoaded", () => {

});