
var ctx = document.getElementById("chart");
const spinner = document.getElementById("spinner");
const spinnerCaption = document.getElementById('spinnerCaption');
let sock;
let settings;


const list = document.getElementById('parameterList');

var dialog = new bootstrap.Modal(document.getElementById('modalOkay'), {backdrop: 'static'});
const dialogTextOkay = document.getElementById('modalBodyId');

document.getElementById('continutBtn').addEventListener('click',()=>{
    window.utils.goToState("results.html");
});


var errorDialog = new bootstrap.Modal(document.getElementById('modalError'), {backdrop: 'static'});
const errorDialogText = document.getElementById('ModalErrorBody')

var crashDialog = new bootstrap.Modal(document.getElementById('modalErrorServerCrash'),{backdrop: 'static'})
const crashDialogText = document.getElementById('modalErrorServerCrashBody')



var myChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [],
        datasets: [{
            label: 'Data',
            data: [],
            borderColor: 'rgb(75, 192, 192)',
            tension: 0.1
        }]
    },
    options: {
        scales: {
            y: {
                beginAtZero: true,
                min:0,
                max:1
            }
        },
        plugins: {
            tooltip: {
                mode: 'index',
                intersect: false,
                callbacks: {
                    label: function(context) {
                        var label = context.dataset.label || '';
                        if (label) {
                            label += ': ';
                        }
                        if (context.parsed.y !== null) {
                            label += context.parsed.y;
                        }
                        return label;
                    }
                }
            }
        }

    }
});



function additem(itemId, Itemname, Itemvalue) {
    const existingItem = list.querySelector(`#${itemId}`);
    if (existingItem) {
        updateItemValue(itemId, Itemvalue);
    } else {
        const newItem = document.createElement('li');
        newItem.className = 'list-group-item';
        newItem.id = itemId;
        newItem.innerHTML = `
            <div class="d-flex justify-content-between align-items-center">
                <span id = "${itemId}_name">${Itemname}:</span>
                <span id = "${itemId}_value">${Itemvalue}</span>
            </div>
        `;
        const list = document.getElementById('parameterList');
        list.appendChild(newItem);
    }
}

function updateItemValue(itemId, newItemValue) {
    const itemValueToUpdate = document.getElementById(`${itemId}_value`);
    itemValueToUpdate.textContent = newItemValue;
}

function processSimulationData(jsonData) {
    if (!Array.isArray(jsonData.parameters) || !jsonData.value || isNaN(parseFloat(jsonData.value))) {
        console.error("Invalid data format:", jsonData);
        return;
    }
    var varName = jsonData.parameters.map(parameter => parameter.name);
    var varValue = jsonData.parameters.map(parameter => parameter.value);
    var varUnit = jsonData.parameters.map(parameter => parameter.unit);
    var value = jsonData.value;

    myChart.data.labels.push(myChart.data.labels.length + 1);
    myChart.data.datasets[0].data.push(parseFloat(value));
    myChart.update();

    for (let i = 0; i < varName.length; i++) {        
        itemId = 'element_' + varName[i].split('.').pop();
        additem(itemId, varName[i].split('.').pop(), `${varValue[i]} ${varUnit[i]}`);
    }
    additem("optVlaueID", "Current optimum", value);
}
  
function processResults(data) {
    localStorage.setItem("resultsData", JSON.stringify(data));
    dialogTextOkay.innerText="The simulation was completed."
    dialog.show()
}
  
function processServerMessage(data) {
    console.log("Processing Server Message..."+data);
    if (data.code == 200){
        spinnerCaption.innerHTML = data.text;
    }else{
        spinnerCaption.innerHTML = "Error " + data.text;
        crashDialogText.innerText = data.text;
        crashDialog.show();
        console.error(data.text);
    }
}
  
function processJSONObject(jsonObject) {
    switch (jsonObject.type) {
      case "SimulationData":
        processSimulationData(jsonObject.data);
        break;
      case "ResultOverview":
        processResults(jsonObject.data);
        break;
      case "ServerMessage":
        processServerMessage(jsonObject.data);
        break;
      default:
        console.error("Unknown type:", jsonObject.type);
    }
}

function removeAbsolutePath(jsonObj) {
    for (let key in jsonObj) {
        if (jsonObj.hasOwnProperty(key)) {
            const entry = jsonObj[key];
            if (entry.hasOwnProperty('params')) {
                entry.params = extractFileName(entry.params);
            }
            if (entry.hasOwnProperty('config')) {
                entry.config = extractFileName(entry.config);
            }
        }
    }
    return jsonObj;
}

function extractFileName(path) {
    const parts = path.split('/');
    return parts[parts.length - 1];
}

let count = 0;
function processBinaryData(data) {
    console.log("Bianry data rec.");
    window.utils.saveZipFile(data,count++);
}

function openWebSocket(uuid) {
    try {
        const url = new URL('/ws', `ws://${settings.serverPath}:${settings.port}`);
        sock = new WebSocket(url);
        
        sock.onopen = () => {
            console.log("WebSocket connection established.");
            additem("startTimeID", "Start time", new Date().toLocaleString());
            spinner.style.display = "inline-block";
            sock.binaryType = "arraybuffer";
            sock.send(uuid);
            sock.send("start");
        };
        
        sock.onclose = () => {
            console.log("WebSocket connection closed.");
            spinner.style.display = "none";
        };
        
        sock.onmessage = (e) => {
            try {
                if (typeof e.data === "string") {
                    var jsonData = JSON.parse(e.data);
                    processJSONObject(jsonData);
                } else if (e.data instanceof ArrayBuffer) {
                    processBinaryData(e.data);
                }
            } catch (error) {
                console.error("WebSocket message processing error:", error);
            }
        };
        
        sock.onerror = (error) => {
            console.error("WebSocket error:", error);
        };

    } catch (error) {
        console.error("Error in openWebSocket function:", error);
        errorDialogText.innerText = `Websocket cant be estabished: ${error}`
        errorDialog.show();
    }
}

document.getElementById('cancelBtn').addEventListener('click',()=>{
    if (sock.readyState < sock.CLOSING){
        sock.send('end')
        console.log("Connection will be closed...")
    }
});
document.getElementById('ModalCrashBtn').addEventListener('click', ()=>{
    if (sock) {
        sock.close();
        sock = null;
    }
    window.utils.goToState("index.html");
});

document.getElementById('ModalErrorBtnCancel').addEventListener('click',()=>{
    if (sock) {
        sock.close();
        sock = null;
    }
    window.utils.goToState("index.html");
});


function executeFetch() {
    spinnerCaption.innerHTML = "Send Project to Server...";
    window.utils.getProjectData().then(data =>{
        const url = new URL('/setSimulationData', `http://${settings.serverPath}:${settings.port}`);
        data = removeAbsolutePath(data);
        fetch(url, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(data)
        })
        .then(response => {
            if (!response.ok) {
                errorDialogText.innerText = `HTTP error! Status: ${response.status}`;
                errorDialog.show();
            }
            return response.text();
        })
        .then(uuid => {
            localStorage.setItem('uuid', uuid);
            console.log('UUID saved:', uuid);
            openWebSocket(uuid);
        })
        .catch(error => {
            errorDialogText.innerText = `HTTP error! Status: ${error}`
            errorDialog.show();
        });
    });
}

document.getElementById('ModalErrorBtnRetry').addEventListener('click',()=>{
    errorDialog.hide();
    setTimeout(() => {
        executeFetch();
    }, 1000);
});

document.addEventListener('DOMContentLoaded', () => {
    settings = window.utils.getSettings();
    executeFetch();
});