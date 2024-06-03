const information = document.getElementById('info')
information.innerText = `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`

document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
})

document.getElementById('newProjectBtn').addEventListener('click',()=>{
    window.utils.goToState('NewProject.html')
});

document.getElementById('newSimulationBtn').addEventListener('click',()=>{

});

document.getElementById('resultsBtn').addEventListener('click',()=>{
    window.utils.goToState('resultmanagment.html')
});

document.getElementById('settingsBtn').addEventListener('click',()=>{
    window.utils.goToState('settings.html')
});


document.addEventListener("DOMContentLoaded",()=> {

});