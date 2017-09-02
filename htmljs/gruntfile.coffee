'use strict'

module.exports = (grunt) ->
  grunt.loadNpmTasks 'grunt-combo-html-css-js'
  grunt.loadNpmTasks 'grunt-contrib-compress'
  grunt.loadNpmTasks 'grunt-contrib-copy'
  grunt.loadNpmTasks 'grunt-contrib-htmlmin'
  grunt.loadNpmTasks 'grunt-contrib-jshint'
  grunt.loadNpmTasks 'grunt-contrib-sass'
  grunt.loadNpmTasks 'grunt-contrib-uglify'
  grunt.loadNpmTasks 'grunt-contrib-watch'
  grunt.loadNpmTasks 'grunt-postcss'

  grunt.initConfig

    copy:
      jsfiles:
        files: [{
          expand: true
          cwd: './src/js/'
          src: '*.js'
          dest: './build/'
        }]

    htmlmin:
      dist:
        options:
          removeComments: true,
          collapseWhitespace: true,
          collapseBooleanAttributes: true,
          removeAttributeQuotes: true,
          removeRedundantAttributes: true,
          removeEmptyAttributes: true,
          minifyJS: true,
          minifyCSS: true
        files: [{
          'dist/index.htm': 'dist/index.htm',
          'dist/control.htm': 'dist/control.htm'
        }]
      dev:
        options:
          removeComments: false,
          collapseWhitespace: false,
          collapseBooleanAttributes: false,
          removeAttributeQuotes: false,
          removeRedundantAttributes: false,
          removeEmptyAttributes: false,
          minifyJS: false,
          minifyCSS: false
        files: [{
          'build/index.html': 'src/index.html',
          'build/control.html': 'src/control.html'
        }]

    comboall:
      main:
        files: [
            { 'dist/index.htm': ['build/index.html'] },
            { 'dist/control.htm': ['build/control.html'] }
        ]

    jshint:
      # In case there is a /dist/ directory, we don't want to lint that
      # so we use the ! (bang) operator to ignore the specified directory
      files: [
        './*.js'
        '!dist/**'
      ]
      options:
        curly: true
        eqeqeq: true
        immed: true
        latedef: true
        newcap: true
        noarg: true
        sub: true
        undef: true
        boss: true
        eqnull: true
        browser: true
        globals:
          console: true

    sass:
      dev:
        options:
          style: 'expanded'
        expand: true
        cwd: 'src/styles/'
        src: [ '*.scss' ]
        dest: './build/'
        ext: '.css'

    postcss:
      options:
        map: true
        processors: [ require('autoprefixer') ]
      dist:
        src: 'build/*.css'

    compress:
      main:
        options:
          mode: 'gzip'
        expand: true
        files: [{
          expand: true
          src: ['dist/*.htm']
          dest: '.'
          ext: '.htm.gz'
        }]

    watch:
      files: [
        'src/**/*'
      ]
      tasks: 'default'

  grunt.registerTask 'build', [
    #'jshint'
    'copy'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'comboall'
    'htmlmin:dist'
    'compress'
  ]

  grunt.registerTask 'default', [
    #'jshint'
    'copy'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'watch'
  ]
