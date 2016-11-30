/*
 * grunt-contrib-clean
 * http://gruntjs.com/
 *
 * Copyright (c) 2016 Tim Branyen, contributors
 * Licensed under the MIT license.
 */

'use strict';

var async = require('async');
var rimraf = require('rimraf');

module.exports = function(grunt) {

  function clean(filepath, options, done) {
    if (!grunt.file.exists(filepath)) {
      return done();
    }

    // Only delete cwd or outside cwd if --force enabled. Be careful, people!
    if (!options.force) {
      if (grunt.file.isPathCwd(filepath)) {
        grunt.verbose.error();
        grunt.fail.warn('Cannot delete the current working directory.');
        return done();
      } else if (!grunt.file.isPathInCwd(filepath)) {
        grunt.verbose.error();
        grunt.fail.warn('Cannot delete files outside the current working directory.');
        return done();
      }
    }

    grunt.verbose.writeln((options['no-write'] ? 'Not actually cleaning ' : 'Cleaning ') + filepath + '...');
    // Actually delete. Or not.
    if (options['no-write']) {
      return done();
    }
    rimraf(filepath, function (err) {
      if (err) {
        grunt.log.error();
        grunt.fail.warn('Unable to delete "' + filepath + '" file (' + err.message + ').', err);
      }
      done();
    });
  }

  grunt.registerMultiTask('clean', 'Clean files and folders.', function() {
    // Merge task-specific and/or target-specific options with these defaults.
    var options = this.options({
      force: grunt.option('force') === true,
      'no-write': grunt.option('no-write') === true
    });

    var done = this.async();

    // Clean specified files / dirs.
    var files = this.filesSrc;
    async.eachSeries(files, function (filepath, cb) {
      clean(filepath, options, cb);
    }, function (err) {
      grunt.log.ok(files.length + ' ' + grunt.util.pluralize(files.length, 'path/paths') + ' cleaned.');
      done(err);
    });
  });

};
