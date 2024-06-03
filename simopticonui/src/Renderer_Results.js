
let projectDir;

document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
});

document.getElementById('contBtn').addEventListener('click',()=>{
    window.utils.goToState("index.html");
});


function loadImagesIntoCarousel(imageFiles) {
    const carouselInner = document.getElementById('carouselContentID');
    carouselInner.innerHTML = '';

    imageFiles.forEach((imageFile, index) => {
        const carouselItem = document.createElement('div');
        carouselItem.classList.add('carousel-item');
        if (index === 0) {
            carouselItem.classList.add('active');
        }
        const rowDiv = document.createElement('div');
        rowDiv.classList.add('row');

        const colLeft = document.createElement('div');
        colLeft.classList.add('col-lg-2');

        const colRight = document.createElement('div');
        colRight.classList.add('col-lg-2');

        const colCenter = document.createElement('div');
        colCenter.classList.add('col-lg-8', 'd-flex', 'justify-content-center', 'align-items-center');

        const imageElement = document.createElement('img');
        imageElement.src = `file://${imageFile}`;
        imageElement.classList.add('d-block', 'w-75', 'img-fluid');
        imageElement.alt = '...';

        colCenter.appendChild(imageElement);
        rowDiv.appendChild(colLeft);
        rowDiv.appendChild(colCenter);
        rowDiv.appendChild(colRight);
        carouselItem.appendChild(rowDiv);
        carouselInner.appendChild(carouselItem);
    });
}

const dropdownMenu = document.getElementById('runSelection');

dropdownMenu.addEventListener('click', event => {
    const target = event.target;
    if (target && target.tagName === 'A') {
        const selectedSimulation = target.textContent;
        const plotsFolderPath = `${projectDir}/${selectedSimulation}/plot`;
        document.getElementById("headerCaption").innerHTML=`Plots for ${selectedSimulation}`
        window.utils.getImages(plotsFolderPath).then(response => {
            if (response.success) {
                const imageFiles = response.imagePaths;
                loadImagesIntoCarousel(imageFiles);
            } else {
                console.error('Error loading images:', response.error);
            }
        });
    }
});

window.addEventListener('DOMContentLoaded', event => {
    window.utils.getProjectData().then(result => {
        projectDir = result.projectDir;

        window.utils.getSimulations(projectDir).then(res =>{ 
            if (res.success && res.simulations.length > 0) {
                const plotsFolderPath = `${projectDir}/${res.simulations[0]}/plot`;
                dropdownMenu.innerHTML = ''; 
                document.getElementById("headerCaption").innerHTML=`Plots for ${res.simulations[0]}`
                res.simulations.forEach(simulation => {
                    const listItem = document.createElement('li');
                    const link = document.createElement('a');
                    link.classList.add('dropdown-item');
                    link.href = '#';
                    link.textContent = simulation;
                    listItem.appendChild(link);
                    dropdownMenu.appendChild(listItem);
                });

                window.utils.getImages(plotsFolderPath).then(response =>{
                    if (response.success) {
                        const imageFiles = response.imagePaths;
                        loadImagesIntoCarousel(imageFiles);
                    } else {
                        console.error('Error loading images:', response.error);
                    }
                });
            }
        });
    });

    const resultData = JSON.parse(localStorage.getItem("resultsData"));
    const datatablesSimple = document.getElementById('datatablesSimple');
    if (datatablesSimple && resultData) {
        const formatParameter = (paramName, paramValue, paramUnit) => {
            const parts = paramName.split('.').pop().split('/');
            const paramNameWithoutPrefix = parts[parts.length - 1];
            return `${paramNameWithoutPrefix} = ${paramValue} ${paramUnit}`;
        };
        const newData = {
            headings: ["Rank", "Value","Parameter", "Path"],
            data: resultData.run.map(item => [
                item.rank,
                item.value,
                item.parameters.map(param => formatParameter(param.name, param.value, param.unit)).join("<br>"),
                item.path
            ])
        };
        datatable = new simpleDatatables.DataTable(datatablesSimple);
        datatable.selectable = false;
        datatable.insert(newData);
        datatable.on("datatable.selectrow", (rowIndex, event) => {
            event.preventDefault();
        });
    }
});



