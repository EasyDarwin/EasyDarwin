# grunt-contrib-clean v0.7.0 [![Build Status: Linux](https://travis-ci.org/gruntjs/grunt-contrib-clean.svg?branch=master)](https://travis-ci.org/gruntjs/grunt-contrib-clean) [![Build Status: Windows](https://ci.appveyor.com/api/projects/status/li28411ceq3n833d/branch/master?svg=true)](https://ci.appveyor.com/project/gruntjs/grunt-contrib-clean/branch/master)

> Clean files and folders



## Getting Started

If you haven't used [Grunt](http://gruntjs.com/) before, be sure to check out the [Getting Started](http://gruntjs.com/getting-started) guide, as it explains how to create a [Gruntfile](http://gruntjs.com/sample-gruntfile) as well as install and use Grunt plugins. Once you're familiar with that process, you may install this plugin with this command:

```shell
npm install grunt-contrib-clean --save-dev
```

Once the plugin has been installed, it may be enabled inside your Gruntfile with this line of JavaScript:

```js
grunt.loadNpmTasks('grunt-contrib-clean');
```

*This plugin was designed to work with Grunt 0.4.x. If you're still using grunt v0.3.x it's strongly recommended that [you upgrade](http://gruntjs.com/upgrading-from-0.3-to-0.4), but in case you can't please use [v0.3.2](https://github.com/gruntjs/grunt-contrib-clean/tree/grunt-0.3-stable).*



## Clean task
_Run this task with the `grunt clean` command._

Task targets, files and options may be specified according to the grunt [Configuring tasks](http://gruntjs.com/configuring-tasks) guide.

*Due to the destructive nature of this task, always be cautious of the paths you clean.*
### Options

#### force
Type: `Boolean`  
Default: `false`

This overrides this task from blocking deletion of folders outside current working dir (CWD). Use with caution.

#### no-write
Type: `Boolean`  
Default: `false`

Will not actually delete any files or directories.
If the task is run with the `--verbose` flag, the task will log messages of what files would have be deleted.

_Note: As this task property contains a hyphen, you will need to surround it with quotes._

### Usage Examples

There are three formats you can use to run this task.

#### Short

```js
clean: ["path/to/dir/one", "path/to/dir/two"]
```

#### Medium (specific targets with global options)

```js
clean: {
  build: ["path/to/dir/one", "path/to/dir/two"],
  release: ["path/to/another/dir/one", "path/to/another/dir/two"]
},
```

#### Long (specific targets with per target options)

```js
clean: {
  build: {
    src: ["path/to/dir/one", "path/to/dir/two"]
  }
}
```

"Compact" and "Files Array" formats support a few [additional properties](http://gruntjs.com/configuring-tasks#files)
which help you deal with hidden files, process dynamic mappings and so on.

#### Globbing Patterns

Although documented [in the Grunt Docs](http://gruntjs.com/configuring-tasks#globbing-patterns), here are some globbing pattern examples to achieve some common tasks:

```js
clean: {
  folder: ['path/to/dir/'],
  folder_v2: ['path/to/dir/**'],
  contents: ['path/to/dir/*'],
  subfolders: ['path/to/dir/*/'],
  css: ['path/to/dir/*.css'],
  all_css: ['path/to/dir/**/*.css']
}
```

* __`folder`:__ Deletes the `dir/` folder
* __`folder_v2`:__ Deletes the `dir/` folder
* __`contents`:__ Keeps the `dir/` folder, but deletes the contents
* __`subfolders`:__ Keeps the files inside the `dir/` folder, but deletes all subfolders
* __`css`:__ Deletes all `*.css` files inside the `dir/` folder, excluding subfolders
* __`all_css`:__ Deletes all `*.css` files inside the `dir/` folder and its subfolders

##### Skipping Files

```js
// Deletes all .js files, but skips min.js files
clean: {
  js: ["path/to/dir/*.js", "!path/to/dir/*.min.js"]
}
```

###### Options

Options can be specified for all `clean` tasks and for each `clean:target`.

####### All tasks

```js
// Prevents all targets from deleting any files
clean: {
  options: {
    'no-write': true
  },
  build: ['dev/build'],
  release: ['dist']
}
```

####### Per-target

```js
// Will delete files for `build` target
// Will NOT delete files for `release` target
clean: {
  build: ['dev/build'],
  release: {
    options: {
      'no-write': true
    },
    src: ['dist']
  }
}
```


## Release History

 * 2016-02-15   v1.0.0   Drop support for Node.js v0.8 Grunt peer dependency tagged `>= 0.4.5` Dependency updates
 * 2015-11-13   v0.7.0   Dependency updates
 * 2014-07-27   v0.6.0   Less verbose output. README updates.
 * 2013-07-15   v0.5.0   Use rimraf directly, version 2.2.1 to fix issue on Windows. Add no-write option to mimic grunt.file.delete behavior.
 * 2013-04-16   v0.4.1   Check if file exists to avoid trying to delete a non-existent file.
 * 2013-02-15   v0.4.0   First official release for Grunt 0.4.0.
 * 2013-01-18   v0.4.0rc6   Updating grunt/gruntplugin dependencies to rc6. Changing in-development grunt/gruntplugin dependency versions from tilde version ranges to specific versions.
 * 2013-01-09   v0.4.0rc5   Updating to work with grunt v0.4.0rc5. Switching to this.filesSrc api.
 * 2012-12-07   v0.4.0a   Conversion to grunt v0.4 conventions. Remove node v0.6 and grunt v0.3 support. Add force option to bypass CWD check.
 * 2012-09-23   v0.3.0   Options no longer accepted from global config key.
 * 2012-09-10   v0.2.0   Refactored from grunt-contrib into individual repo.

---

Task submitted by [Tim Branyen](http://tbranyen.com/)

*This file was generated on Mon Feb 15 2016 13:42:25.*
