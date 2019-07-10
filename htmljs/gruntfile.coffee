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
  grunt.loadNpmTasks 'grunt-processhtml'
  grunt.loadNpmTasks 'grunt-multi-lang-site-generator'

  grunt.initConfig

    copy:
      jsfiles:
        files: [{
          expand: true
          cwd: './src/js'
          src: '**/*.js'
          dest: './build/js/'
        }]

    htmlmin:
      dist:
        options:
          removeComments: true,
          collapseWhitespace: true,
          collapseBooleanAttributes: true,
          removeAttributeQuotes: true,
          removeRedundantAttributes: false,
          removeEmptyAttributes: true,
          minifyJS: true,
          minifyCSS: true
        files: [{
          'dist/index.tmpl.htm': 'dist/index.tmpl.htm',
          'dist/index_s.tmpl.htm': 'dist/index_s.tmpl.htm',
          'dist/control.tmpl.htm': 'dist/control.tmpl.htm',
          'dist/control_s.tmpl.htm': 'dist/control_s.tmpl.htm',
          'dist/setup.tmpl.htm': 'dist/setup.tmpl.htm',
          'dist/gravity.tmpl.htm': 'dist/gravity.tmpl.htm',
          'dist/logging.tmpl.htm': 'dist/logging.tmpl.htm',
          'dist/config.tmpl.htm': 'dist/config.tmpl.htm',
          'dist/pressure.tmpl.htm': 'dist/pressure.tmpl.htm',
          'dist/classic-index.tmpl.htm': 'dist/classic-index.tmpl.htm',
          'dist/classic-setup.tmpl.htm': 'dist/classic-setup.tmpl.htm',
          'dist/classic-gdc.tmpl.htm': 'dist/classic-gdc.tmpl.htm',
          'dist/classic-log.tmpl.htm': 'dist/classic-log.tmpl.htm',
          'dist/classic-config.tmpl.htm': 'dist/classic-config.tmpl.htm',
          'dist/BPLLogViewer.tmpl.htm': 'dist/BPLLogViewer.tmpl.htm',
          'dist/BPLog.tmpl.htm': 'dist/BPLog.tmpl.htm',
          'dist/lcd.htm': 'dist/lcd.htm',
          'dist/classic-lcd.htm': 'dist/classic-lcd.htm'
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
          'build/index.tmpl.html': 'build/index.tmpl.html',
          'build/index_s.tmpl.html': 'build/index_s.tmpl.html',
          'build/control.tmpl.html': 'build/control.tmpl.html',
          'build/control_s.tmpl.html': 'build/control_s.tmpl.html',
          'build/setup.tmpl.html': 'build/setup.tmpl.html',
          'build/gravity.tmpl.html': 'build/gravity.tmpl.html',
          'build/logging.tmpl.html': 'build/logging.tmpl.html',
          'build/config.tmpl.html': 'build/config.tmpl.html',
          'build/pressure.tmpl.html': 'build/pressure.tmpl.html',
          'build/classic-index.tmpl.html': 'build/classic-index.tmpl.html',
          'build/classic-setup.tmpl.html': 'build/classic-setup.tmpl.html',
          'build/classic-gdc.tmpl.html': 'build/classic-gdc.tmpl.html',
          'build/classic-log.tmpl.html': 'build/classic-log.tmpl.html',
          'build/classic-config.tmpl.html': 'build/classic-config.tmpl.html',
          'build/BPLLogViewer.tmpl.html': 'build/BPLLogViewer.tmpl.html',
          'build/BPLog.tmpl.html': 'build/BPLog.tmpl.html',
          'build/lcd.html': 'build/lcd.html',
          'build/classic-lcd.html': 'build/classic-lcd.html'
        }]

    comboall:
      main:
        files: [
            { 'dist/index.tmpl.htm': ['build/index.tmpl.html'] },
            { 'dist/index_s.tmpl.htm': ['build/index_s.tmpl.html'] },
            { 'dist/control.tmpl.htm': ['build/control.tmpl.html'] },
            { 'dist/control_s.tmpl.htm': ['build/control_s.tmpl.html'] },
            { 'dist/setup.tmpl.htm': ['build/setup.tmpl.html'] },
            { 'dist/gravity.tmpl.htm': ['build/gravity.tmpl.html'] },
            { 'dist/logging.tmpl.htm': ['build/logging.tmpl.html'] },
            { 'dist/config.tmpl.htm': ['build/config.tmpl.html'] },
            { 'dist/pressure.tmpl.htm': ['build/pressure.tmpl.html'] },
            { 'dist/classic-index.tmpl.htm': ['build/classic-index.tmpl.html'] },
            { 'dist/classic-setup.tmpl.htm': ['build/classic-setup.tmpl.html'] },
            { 'dist/classic-gdc.tmpl.htm': ['build/classic-gdc.tmpl.html'] },
            { 'dist/classic-log.tmpl.htm': ['build/classic-log.tmpl.html'] },
            { 'dist/classic-config.tmpl.htm': ['build/classic-config.tmpl.html'] },
            { 'dist/BPLLogViewer.tmpl.htm': ['build/BPLLogViewer.tmpl.html'] },
            { 'dist/BPLog.tmpl.htm': ['build/BPLog.tmpl.html'] },
            { 'dist/lcd.htm': ['build/lcd.html']},
            { 'dist/classic-lcd.htm': ['build/classic-lcd.html']}
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
          level: 9
        expand: true
        files: [{
          expand: true
          src: ['dist/english/*.htm','dist/chinese/*.htm','dist/spanish/*.htm','dist/portuguese-br/*.htm','dist/slovak/*.htm','dist/lcd.htm','dist/classic-lcd.htm']
          dest: '.'
          ext: '.htm.gz'
        }]

    processhtml:
      dist:
        files: [
          'build/index.tmpl.html': ['src/index.tmpl.html']
          'build/index_s.tmpl.html': ['src/index_s.tmpl.html']
          'build/control.tmpl.html': ['src/control.tmpl.html']
          'build/control_s.tmpl.html': ['src/control_s.tmpl.html']
          'build/setup.tmpl.html': ['src/setup.tmpl.html']
          'build/gravity.tmpl.html': ['src/gravity.tmpl.html']
          'build/logging.tmpl.html': ['src/logging.tmpl.html']
          'build/config.tmpl.html': ['src/config.tmpl.html'],          
          'build/pressure.tmpl.html': ['src/pressure.tmpl.html'],          
          'build/classic-index.tmpl.html': ['src/classic-index.tmpl.html']
          'build/classic-setup.tmpl.html': ['src/classic-setup.tmpl.html']
          'build/classic-gdc.tmpl.html': ['src/classic-gdc.tmpl.html']
          'build/classic-log.tmpl.html': ['src/classic-log.tmpl.html']
          'build/classic-config.tmpl.html': ['src/classic-config.tmpl.html']
          'build/BPLLogViewer.tmpl.html': ['src/BPLLogViewer.tmpl.html']
          'build/BPLog.tmpl.html': ['src/BPLog.tmpl.html']
          'build/lcd.html' : ['src/lcd.html']
          'build/classic-lcd.html' : ['src/classic-lcd.html']
        ]
  
    multi_lang_site_generator:
      default:
          options:
            vocabs:           ['english','chinese','spanish','portuguese-br', 'slovak']
            vocab_directory:  'src/locales'
            output_directory: 'dist'
            template_directory: 'dist'
          files: [
            'index.htm': ['index.tmpl.htm']
            'index_s.htm': ['index_s.tmpl.htm']
            'control.htm': ['control.tmpl.htm']
            'control_s.htm': ['control_s.tmpl.htm']
            'setup.htm': ['setup.tmpl.htm']
            'gravity.htm': ['gravity.tmpl.htm']
            'logging.htm': ['logging.tmpl.htm']
            'config.htm': ['config.tmpl.htm']
            'pressure.htm': ['pressure.tmpl.htm']
            'classic-index.htm': ['classic-index.tmpl.htm']
            'classic-setup.htm': ['classic-setup.tmpl.htm']
            'classic-gdc.htm': ['classic-gdc.tmpl.htm']
            'classic-log.htm': ['classic-log.tmpl.htm']
            'classic-config.htm': ['classic-config.tmpl.htm']
            'BPLog.htm': ['BPLog.tmpl.htm']
            'BPLLogViewer.htm': ['BPLLogViewer.tmpl.htm']
          ]


    watch:
      files: [
        'src/**/*'
      ]
      tasks: 'default'

  grunt.registerTask 'build', [
    #'jshint'
    'copy'
    'processhtml'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'comboall'
    'htmlmin:dist'
    'compress'
  ]

  grunt.registerTask 'debug', [
    #'jshint'
    'copy'
    'processhtml'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'comboall'
    'multi_lang_site_generator'
    'compress'
  ]

  grunt.registerTask 'default', [
    #'jshint'
    'copy'
    'processhtml'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'watch'
  ]


  grunt.registerTask 'i18n', [
    'copy'
    'processhtml'
    'htmlmin:dev'
    'sass:dev'
    'postcss'
    'comboall'
    'htmlmin:dist'
    'multi_lang_site_generator'
    'compress'
  ]
