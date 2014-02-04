// Copyright Natsume23
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

#include <node.h>
#include <node_androidlog.h>

#include <v8.h>

#include <string.h>

#define DEBUG
#ifdef DEBUG
# ifdef ANDROID
#  include <android/log.h>
#  define DEBUG_TAG "node.js"
#  define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, DEBUG_TAG, __VA_ARGS__)
#  define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,   DEBUG_TAG, __VA_ARGS__)
#  define LOGI(...) __android_log_print(ANDROID_LOG_INFO,    DEBUG_TAG, __VA_ARGS__)
#  define LOGW(...) __android_log_print(ANDROID_LOG_WARN,    DEBUG_TAG, __VA_ARGS__)
#  define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,   DEBUG_TAG, __VA_ARGS__)
# else
#  define LOGV(...) fprintf(stdout, __VA_ARGS__)
#  define LOGD(...) fprintf(stdout, __VA_ARGS__)
#  define LOGI(...) fprintf(stdout, __VA_ARGS__)
#  define LOGW(...) fprintf(stderr, __VA_ARGS__)
#  define LOGE(...) fprintf(stderr, __VA_ARGS__)
# endif
#else
# define LOGV(...)
# define LOGD(...)
# define LOGI(...)
# define LOGW(...)
# define LOGE(...)
#endif

namespace node {

using namespace v8;

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}


static Handle<Value> Verbose(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() > 0) {
    v8::String::Utf8Value msg(args[0]);  
    LOGV(ToCString(msg));
  }
  return scope.Close(Undefined());
}

static Handle<Value> Debug(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() > 0) {
    v8::String::Utf8Value msg(args[0]);  
    LOGD(ToCString(msg));
  }
  return scope.Close(Undefined());
}

static Handle<Value> Info(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() > 0) {
    v8::String::Utf8Value msg(args[0]);  
    LOGI(ToCString(msg));
  }
  return scope.Close(Undefined());
}

static Handle<Value> Warn(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() > 0) {
    v8::String::Utf8Value msg(args[0]);  
    LOGW(ToCString(msg));
  }
  return scope.Close(Undefined());
}

static Handle<Value> Error(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() > 0) {
    v8::String::Utf8Value msg(args[0]);  
    LOGE(ToCString(msg));
  }
  return scope.Close(Undefined());
}

void Androidlog::Initialize(v8::Handle<v8::Object> target) {
  HandleScope scope;

  NODE_SET_METHOD(target, "log", Verbose);
  NODE_SET_METHOD(target, "verbose", Verbose);
  NODE_SET_METHOD(target, "debug", Debug);
  NODE_SET_METHOD(target, "info", Info);
  NODE_SET_METHOD(target, "warn", Warn);
  NODE_SET_METHOD(target, "error", Error);
}

}  // namespace node

NODE_MODULE(node_androidlog, node::Androidlog::Initialize)
