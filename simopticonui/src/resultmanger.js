
const datatablesSimple = document.getElementById('datatablesSimple');
const dataTable = new simpleDatatables.DataTable(datatablesSimple);

document.getElementById('info').innerText =
    `Using Chrome (v${window.versions.chrome()}), Node.js (v${window.versions.node()}), and Electron (v${window.versions.electron()})`


document.getElementById('backBtn').addEventListener('click',()=>{
    window.utils.goBack();
})

document.getElementById('closeBtn').addEventListener('click',()=>{
    window.close();
})

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

function loadData(){
    window.utils.listProjects().then(result => {
        console.log(result)
        if (datatablesSimple && result) {

            const newData = {
                headings: ["Project", "Path","Modifed", "Has plots"],
                data: result.map(item => [
                    item.name,
                    item.path,
                    item.modified,
                    "False"
                ])
            };
            
            dataTable.selectable = true;
            dataTable.insert(newData);
            dataTable.on("datatable.selectrow", (rowIndex, event) => {
                event.preventDefault();
                projectPath = dataTable.data.data.at(rowIndex).at(1).text
                console.log(projectPath)
                const plotsFolderPath = `${projectPath}/0/plot`;
                document.getElementById("headerCaption").innerHTML=`Plots for ${plotsFolderPath}`
                window.utils.getImages(plotsFolderPath).then(response => {
                    if (response.success) {
                        const imageFiles = response.imagePaths;
                        loadImagesIntoCarousel(imageFiles);
                    } else {
                        console.error('Error loading images:', response.error);
                    }
                });
            });
        }

    });       
}

window.addEventListener('DOMContentLoaded', event => {
    loadData();
});
