# Userinterface für Simopticon

## Installation of Electron.js
The best way to install NodeJS is through their website using a package manager.


https://nodejs.org/en/download/package-manager

This app has been built and tested with version 20.13.1 LTS. 
Attention: Ubuntu has deprecated NodeJS modules in their repositories. Do not use them!

```
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
nvm install 20
```
## Prepareing the code
After cloning the project, the Electron Forge project must be initialized as follows:
```bash
cd simopticonui
npm install --save-dev @electron-forge/cli
npm exec --package=@electron-forge/cli -c "electron-forge import"
```
## Running the code
```bash
npn start
```
