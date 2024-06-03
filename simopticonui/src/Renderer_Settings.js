const information = document.getElementById('info')
information.innerText = `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`

let settings;

document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
})

function loadSettings(jsonObj) {
    document.getElementById('sumoPath').value = jsonObj.sumoPath;
    document.getElementById('plexePath').value = jsonObj.plexePath;
    document.getElementById('startServerLocal').checked = jsonObj.startServerLocal;
    document.getElementById('serverPath').value = jsonObj.serverPath;
    document.getElementById('projectFilePath').value = jsonObj.projectPath;
    document.getElementById('port').value = jsonObj.port;
}


document.addEventListener("DOMContentLoaded", function () {
    settings =  window.utils.getSettings();
    loadSettings(settings);
    var startServerLocal = document.getElementById('startServerLocal');
    var pathLabel = document.getElementById('pathLabel');

    startServerLocal.addEventListener('change', function () {
        if (startServerLocal.checked) {
            pathLabel.textContent = "Pfad zu Lokalen Server";
        } else {
            pathLabel.textContent = "Pfad zum Remoteserver:";
        }
    });
});

document.getElementById("saveSettingsBtn").addEventListener('click',()=>{
    var jsonObj = {
        sumoPath: document.getElementById('sumoPath').value,
        plexePath: document.getElementById('plexePath').value,
        startServerLocal: document.getElementById('startServerLocal').checked,
        serverPath: document.getElementById('serverPath').value,
        port: document.getElementById('port').value,
        projectPath: document.getElementById('projectFilePath').value
    };
    window.utils.saveSettings(jsonObj);
});

document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
})
