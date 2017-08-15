'use strict'

module.exports = (grunt) ->
  grunt.loadNpmTasks 'grunt-combo-html-css-js'
  grunt.loadNpmTasks 'grunt-contrib-copy'
  grunt.loadNpmTasks 'grunt-contrib-htmlmin'
  grunt.loadNpmTasks 'grunt-contrib-jshint'
  grunt.loadNpmTasks 'grunt-contrib-sass'
  grunt.loadNpmTasks 'grunt-contrib-watch'

  grunt.initConfig

    copy:
      jsfiles:
        files: [{
          expand: false
          cwd: '.'
          src: 'src/js/*.js'
          dest: 'build/'
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
          'dist/index.html': 'dist/index.html'
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
          'build/index.html': 'src/index.html'
        }]

    comboall:
      main:
        files: [
            {'dist/index.html': ['build/index.html']}
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
        cwd: '.'
        src: [ 'src/styles/*.scss' ]
        dest: './build/'
        ext: '.css'

    watch:
      files: [
        'src/index.html'
        '<%= jshint.files %>'
        '<%= sass.dev.src %>'
      ]
      tasks: 'dev'

  grunt.registerTask 'build', [
    #'jshint'
    'copy'
    'htmlmin:dev'
    'sass:dev'
    'comboall'
    'htmlmin:dist'
  ]

  grunt.registerTask 'dev', [
    #'jshint'
    'copy'
    'htmlmin:dev'
    'sass:dev'
    'watch'
  ]
