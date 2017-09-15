# New Frontend for BPL

If you just want to test, try or use the new layout, please download the dist/ folder and upload the files to your BPL. If you are running the latest version with gzip-support, upload the .gz-files, otherwise the .htm-files

## Developing

For developing / building the project yourself you need to have nodejs installed on your system.

Clone the repository and enter the htmljs/ folder, run

```
npm install
```

to install the dependencies.

### Watch Mode
We use grunt to automate development and building tasks. If you want to test your changes run grunt in watch mode (default) using:
```
grunt
```

### Build
If you want to build the project run:
```
grunt build
```