The web front pages are built by 'grunt'. Please check the readme in the directory for detail.

https://github.com/vitotai/BrewPiLess/tree/master/htmljs

To add localization of the pages, a translation file is necessary. The format of translation file is JSON. A simple and good way to begin is copying the english.json and starting from it.

https://github.com/vitotai/BrewPiLess/tree/master/htmljs/src/locales

It's better to make sure that everything will work before you start translating. So, you can copy the copy the english.json to another file, and run the following procedure first.


 * Put the the file, say  wakanda.json, in "locales" subdirectory.

 * modify gruntfile.coffee. Find around line 167:

``````
    multi_lang_site_generator:
      default:
          options:
            vocabs:           ['english','chinese','spanish']
            vocab_directory:  'src/locales'
            output_directory: 'dist'
````````

Add the language to be added, in the same name as the json file.

``````
    multi_lang_site_generator:
      default:
          options:
            vocabs:           ['english','chinese','spanish', 'wakanda']
            vocab_directory:  'src/locales'
            output_directory: 'dist'
````````

 * Run grunt by
``````
    grunt i18n
``````

 * if everything goes well, there will be a 'wakanda' subdirectory in "dist" path.

If you don't understand this page so far, but you can and are willing to do the translation, just translate the json file and send it to me. (Or, submit an issue with attached translated file, I guess.) 
