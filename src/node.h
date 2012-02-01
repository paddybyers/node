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

#ifndef SRC_NODE_H_
#define SRC_NODE_H_

#ifdef _WIN32
# ifndef BUILDING_NODE_EXTENSION
#   define NODE_EXTERN __declspec(dllexport)
# else
#   define NODE_EXTERN __declspec(dllimport)
# endif
#else
# ifndef BUILDING_NODE_EXTENSION
#   define NODE_EXTERN __attribute__((visibility("default")))
# else
#   define NODE_EXTERN /* nothing */
# endif
#endif

#ifdef BUILDING_NODE_EXTENSION
# undef BUILDING_V8_SHARED
# undef BUILDING_UV_SHARED
# define USING_V8_SHARED 1
# define USING_UV_SHARED 1
#endif

// This should be defined in make system.
// See issue https://github.com/joyent/node/issues/1236
#if defined(__MINGW32__) || defined(_MSC_VER)
#ifndef _WIN32_WINNT
# define _WIN32_WINNT   0x0501
#endif

#define NOMINMAX

#endif

#if defined(_MSC_VER)
#define PATH_MAX MAX_PATH
#endif

#ifdef _WIN32
# define SIGKILL         9
#endif

#include <uv.h>
#include <v8.h>
#include <sys/types.h> /* struct stat */
#include <sys/stat.h>
#include <assert.h>

#include <node_statics.h>
#include <node_object_wrap.h>

#include <limits.h> /* PATH_MAX */

#ifndef offset_of
// g++ in strict mode complains loudly about the system offsetof() macro
// because it uses NULL as the base address.
#define offset_of(type, member) \
  ((intptr_t) ((char *) &(((type *) 8)->member) - 8))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offset_of(type, member)))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

#ifndef NODE_STRINGIFY
#define NODE_STRINGIFY(n) NODE_STRINGIFY_HELPER(n)
#define NODE_STRINGIFY_HELPER(n) #n
#endif

#define HAVE_ISOLATES 1

#ifdef NODE_LIBRARY
# define EXIT(X) node::Isolate *i = node::Isolate::GetCurrent(); if(!i->exit_status) i->exit_status = (X)
# define RETURN_ON_EXIT(X) if(exit_status) { if(exitHandler) exitHandler(); return X; }
# define BREAK_AND_EXIT(X) ev_break(node::Isolate::GetCurrent()->Loop()->ev, EVBREAK_ALL); EXIT(X) //FIXME: implement generically for uv
#else
# define EXIT(X) exit((X))
# define RETURN_ON_EXIT(X)
# define BREAK_AND_EXIT(X) EXIT(X)
#endif // NODE_LIBRARY

namespace node {

class NodeOptions {
public:
  // the index of the first non-option argument, after processing
  int args_start_index;
  // instance options, may be specified globally or per-isolate
  char *eval_string;
  int max_stack_size;
  v8::ResourceConstraints constraints;
  // global-only (debug) options, ignored if passed as isolate options
  bool use_debug_agent;
  bool debug_wait_connect;
  int debug_port;
  void ParseArgs(int argc, char **argv);
  void ParseDebugOpt(const char* arg);
  void SetResourceConstraints();
  NodeOptions();
  ~NodeOptions();
};
    
class Isolate {
public:
    NODE_EXTERN static Isolate* GetDefault();
    NODE_EXTERN static Isolate* GetCurrent();
    static uv_loop_t* GetCurrentLoop();
    int Start(uv_thread_shared_t *options);
    NODE_EXTERN int Start(int argc, char *argv[]);
    NODE_EXTERN int Stop(int signum);
    NODE_EXTERN static Isolate* New();
    NODE_EXTERN void Dispose();
    NODE_EXTERN void setExitHandler(void (*)());

    v8::Local<v8::Value> ErrnoException(int errorno,
        const char *syscall = NULL,
        const char *msg = "",
        const char *path = NULL);
    void __FatalException(v8::TryCatch &try_catch);
    void __SetErrno(uv_err_t err);
    v8::Persistent<v8::Object> local_env;
    NODE_EXTERN uv_loop_t *Loop();
    NODE_EXTERN uv_loop_t *GetLoop();
    ext_statics statics_;
    int exit_status;
    int term_signal;

    Isolate();
    ~Isolate();
    
private:
    void StartGCTimer ();
    void StopGCTimer ();

    static void Idle(uv_idle_t* watcher, int status);
    static void Check(uv_check_t* watcher, int status);
    static void Spin(uv_idle_t* handle, int status);
    static void PrepareTick(uv_prepare_t* handle, int status);
    static void CheckTick(uv_check_t* handle, int status);
    static void CheckStatus(uv_timer_t* watcher, int status);

    void __Idle(uv_idle_t* watcher, int status);
    void __Check(uv_check_t* watcher, int status);
    void __Spin(uv_idle_t* handle, int status);
    void __PrepareTick(uv_prepare_t* handle, int status);
    void __CheckTick(uv_check_t* handle, int status);
    void __CheckStatus(uv_timer_t* watcher, int status);
    void Tick(void);

    static v8::Handle<v8::Value> NeedTickCallback(const v8::Arguments& args);
    static v8::Handle<v8::Value> Chdir(const v8::Arguments& args);
    static v8::Handle<v8::Value> Cwd(const v8::Arguments& args);
    static v8::Handle<v8::Value> Umask(const v8::Arguments& args);
#ifdef __POSIX__
    static v8::Handle<v8::Value> GetUid(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetUid(const v8::Arguments& args);
    static v8::Handle<v8::Value> GetGid(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetGid(const v8::Arguments& args);
#endif // __POSIX__
    static v8::Handle<v8::Value> Exit(const v8::Arguments& args);
    static v8::Handle<v8::Value> MemoryUsage(const v8::Arguments& args);
    static v8::Handle<v8::Value> Kill(const v8::Arguments& args);
    static v8::Handle<v8::Value> Binding(const v8::Arguments& args);

    v8::Handle<v8::Object> GetFeatures();

    int Init(int argc, char *argv[]);
    v8::Handle<v8::Object> SetupProcessObject(int argc, char *argv[]);
    void SetupLocalEnv(char *env[]);
    void Load(v8::Handle<v8::Object> process);
    void EmitExit(v8::Handle<v8::Object> process);

    v8::Isolate *isolate;
    v8::Persistent<v8::Object> process;
    
    v8::Persistent<v8::String> errno_symbol;
    v8::Persistent<v8::String> syscall_symbol;
    v8::Persistent<v8::String> errpath_symbol;
    v8::Persistent<v8::String> code_symbol;
    
    v8::Persistent<v8::String> rss_symbol;
    v8::Persistent<v8::String> heap_total_symbol;
    v8::Persistent<v8::String> heap_used_symbol;
    
    v8::Persistent<v8::String> listeners_symbol;
    v8::Persistent<v8::String> uncaught_exception_symbol;
    v8::Persistent<v8::String> emit_symbol;
    
    NodeOptions options;
    int uncaught_exception_counter;

    uv_check_t check_tick_watcher;
    uv_prepare_t prepare_tick_watcher;
    uv_idle_t tick_spinner;
    uv_async_t stop_watcher;
    
    bool need_tick_cb;
    v8::Persistent<v8::String> tick_callback_sym;

    bool use_npn;
    bool use_sni;
  
    int stdin_fd, stdout_fd, stderr_fd;
    
    // Buffer for getpwnam_r(), getgrpam_r() and other misc callers; keep this
    // scoped at file-level rather than method-level to avoid excess stack usage.
    char getbuf[PATH_MAX + 1];
    
    // We need to notify V8 when we're idle so that it can run the garbage
    // collector. The interface to this is V8::IdleNotification(). It returns
    // true if the heap hasn't be fully compacted, and needs to be run again.
    // Returning false means that it doesn't have anymore work to do.
    //
    // A rather convoluted algorithm has been devised to determine when Node is
    // idle. You'll have to figure it out for yourself.
    uv_check_t gc_check;
    uv_idle_t gc_idle;
    uv_timer_t gc_timer;
    uv_loop_t *loop_;
    
    int tick_time_head;
    
    v8::Persistent<v8::Object> binding_cache;
    v8::Persistent<v8::Array> module_load_list;
    void (*exitHandler)();

};

NODE_EXTERN int Start(int argc, char *argv[]);
NODE_EXTERN int Initialize(int argc, char *argv[]);
NODE_EXTERN void Dispose();

#define NODE_PSYMBOL(s) \
  v8::Persistent<v8::String>::New(v8::String::NewSymbol(s))

/* Converts a unixtime to V8 Date */
#define NODE_UNIXTIME_V8(t) v8::Date::New(1000*static_cast<double>(t))
#define NODE_V8_UNIXTIME(v) (static_cast<double>((v)->NumberValue())/1000.0);

#define NODE_DEFINE_CONSTANT(target, constant)                            \
  (target)->Set(v8::String::NewSymbol(#constant),                         \
                v8::Integer::New(constant),                               \
                static_cast<v8::PropertyAttribute>(                       \
                    v8::ReadOnly|v8::DontDelete))

#define NODE_SET_METHOD(obj, name, callback)                              \
  obj->Set(v8::String::NewSymbol(name),                                   \
           v8::FunctionTemplate::New(callback)->GetFunction())

#define NODE_SET_PROTOTYPE_METHOD(templ, name, callback)                  \
do {                                                                      \
  v8::Local<v8::Signature> __callback##_SIG = v8::Signature::New(templ);  \
  v8::Local<v8::FunctionTemplate> __callback##_TEM =                      \
    v8::FunctionTemplate::New(callback, v8::Handle<v8::Value>(),          \
                          __callback##_SIG);                              \
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol(name),            \
                                  __callback##_TEM);                      \
} while (0)

enum encoding {ASCII, UTF8, BASE64, UCS2, BINARY, HEX};
enum encoding ParseEncoding(v8::Handle<v8::Value> encoding_v,
                            enum encoding _default = BINARY);
NODE_EXTERN void FatalException(v8::TryCatch &try_catch);
void DisplayExceptionLine(v8::TryCatch &try_catch); // hack

v8::Local<v8::Value> Encode(const void *buf, size_t len,
                            enum encoding encoding = BINARY);

// Returns -1 if the handle was not valid for decoding
ssize_t DecodeBytes(v8::Handle<v8::Value>,
                    enum encoding encoding = BINARY);

// returns bytes written.
ssize_t DecodeWrite(char *buf,
                    size_t buflen,
                    v8::Handle<v8::Value>,
                    enum encoding encoding = BINARY);

// Use different stat structs & calls on windows and posix;
// on windows, _stati64 is utf-8 and big file aware.
#if __POSIX__
# define NODE_STAT        stat
# define NODE_FSTAT       fstat
# define NODE_STAT_STRUCT struct stat
#else // _WIN32
# define NODE_STAT        _stati64
# define NODE_FSTAT       _fstati64
# define NODE_STAT_STRUCT struct _stati64
#endif

v8::Local<v8::Object> BuildStatsObject(NODE_STAT_STRUCT *s);


/**
 * Call this when your constructor is invoked as a regular function, e.g.
 * Buffer(10) instead of new Buffer(10).
 * @param constructorTemplate Constructor template to instantiate from.
 * @param args The arguments object passed to your constructor.
 * @see v8::Arguments::IsConstructCall
 */
v8::Handle<v8::Value> FromConstructorTemplate(
    v8::Persistent<v8::FunctionTemplate>& constructorTemplate,
    const v8::Arguments& args);


static inline v8::Persistent<v8::Function>* cb_persist(
    const v8::Local<v8::Value> &v) {
  v8::Persistent<v8::Function> *fn = new v8::Persistent<v8::Function>();
  *fn = v8::Persistent<v8::Function>::New(v8::Local<v8::Function>::Cast(v));
  return fn;
}

static inline v8::Persistent<v8::Function>* cb_unwrap(void *data) {
  v8::Persistent<v8::Function> *cb =
    reinterpret_cast<v8::Persistent<v8::Function>*>(data);
  assert((*cb)->IsFunction());
  return cb;
}

static inline void cb_destroy(v8::Persistent<v8::Function> * cb) {
  cb->Dispose();
  delete cb;
}

NODE_EXTERN v8::Local<v8::Value> ErrnoException(int errorno,
                                                const char *syscall = NULL,
                                                const char *msg = "",
                                                const char *path = NULL);
#ifdef _WIN32
NODE_EXTERN v8::Local<v8::Value> WinapiErrnoException(int errorno,
    const char *syscall = NULL,  const char *msg = "",
    const char *path = NULL);
#endif

const char *signo_string(int errorno);

struct node_module_struct {
  int version;
  void *dso_handle;
  const char *filename;
  void (*register_func) (v8::Handle<v8::Object> target);
  const char *modname;
};

node_module_struct* get_builtin_module(const char *name, int *idx=0);

/**
 * When this version number is changed, node.js will refuse
 * to load older modules.  This should be done whenever
 * an API is broken in the C++ side, including in v8 or
 * other dependencies
 */
#define NODE_MODULE_VERSION (1)

#define NODE_STANDARD_MODULE_STUFF \
          NODE_MODULE_VERSION,     \
          NULL,                    \
          __FILE__

#ifdef _WIN32
# define NODE_MODULE_EXPORT __declspec(dllexport)
#else
# define NODE_MODULE_EXPORT /* empty */
#endif

#define NODE_MODULE(modname, regfunc)                                 \
  extern "C" {                                                        \
    NODE_MODULE_EXPORT node::node_module_struct modname ## _module =  \
    {                                                                 \
      NODE_STANDARD_MODULE_STUFF,                                     \
      regfunc,                                                        \
      NODE_STRINGIFY(modname)                                         \
    };                                                                \
  }

#define NODE_MODULE_DECL(modname) \
  extern "C" node::node_module_struct modname ## _module;

NODE_EXTERN void SetErrno(uv_err_t err);
NODE_EXTERN void SetLastErrno();
NODE_EXTERN void MakeCallback(v8::Handle<v8::Object> object,
                              const char* method,
                              int argc,
                              v8::Handle<v8::Value> argv[]);

}  // namespace node
#endif  // SRC_NODE_H_
