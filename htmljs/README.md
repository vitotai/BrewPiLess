# New Frontend for BPL

If you just want to test, try or use the new layout, please download the dist/ folder and upload the files to your BPL. If you are running the latest version with gzip-support, upload the .gz-files, otherwise the .htm-files

## Developing

For developing / building the project yourself you need to have nodejs and ruby installed on your system.

Clone the repository and enter the htmljs/ folder, run

```
npm install
npm install -g grunt-cli
gem install sass
```

to install the dependencies.

### Watch Mode
We use grunt to automate development and building tasks. If you want to test your changes run grunt in watch mode (default) using:
```
grunt
```
This will automatically recompile the code, you will just need to refresh the browser when it is finished.

### Build
If you want to build the project run:
```
grunt build
```

After v3.1, run:
```
grunt i18n
```


## ToDo

- the "Waiting to Cool, Cooling, Heating"-text, etc on the STATUS could be color coded to match the colors that used to show under the graph
- unified header for all pages
- ~~add better mobile support~~
- ~~missing the heat/cool colors under the graph from the old one~~
- ~~legend jumps for long states~~