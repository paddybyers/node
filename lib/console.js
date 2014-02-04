// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

var util = require('util');


var os = require('os');
var isandroid = (os.platform() == 'android');
if (isandroid) {
  var binding = process.binding('androidlog');
}

if (!isandroid) {
  exports.log = function() {
    process.stdout.write(util.format.apply(this, arguments) + '\n');
  };
} else {
  exports.log = function() {
    binding.debug(util.format.apply(this, arguments));
  };
};


if (!isandroid) {
  exports.info = exports.log;
} else {
  exports.info = function() {
    binding.info(util.format.apply(this, arguments));
  };
};


if (!isandroid) {
  exports.warn = function() {
    process.stderr.write(util.format.apply(this, arguments) + '\n');
  };
} else {
  exports.warn = function() {
    binding.warn(util.format.apply(this, arguments));
  };
};


if (!isandroid) {
  exports.error = exports.warn;
} else {
  exports.error = function() {
    binding.error(util.format.apply(this, arguments));
  };
};


if (!isandroid) {
  exports.dir = function(object) {
    process.stdout.write(util.inspect(object) + '\n');
  };
} else {
  exports.dir = function(object) {
    binding.verbose(util.inspect(object));
  };
};


var times = {};
exports.time = function(label) {
  times[label] = Date.now();
};


exports.timeEnd = function(label) {
  var duration = Date.now() - times[label];
  exports.log('%s: %dms', label, duration);
};


exports.trace = function(label) {
  // TODO probably can to do this better with V8's debug object once that is
  // exposed.
  var err = new Error;
  err.name = 'Trace';
  err.message = label || '';
  Error.captureStackTrace(err, arguments.callee);
  console.error(err.stack);
};


exports.assert = function(expression) {
  if (!expression) {
    var arr = Array.prototype.slice.call(arguments, 1);
    require('assert').ok(false, util.format.apply(this, arr));
  }
};
