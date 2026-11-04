# Building a Softil-style Library (C API over a C/C++ SIP core)

A step-by-step guide to designing a library that exposes a **C API** to the
outside world (like Softil's MCX SDK or PJSIP) while using PJSIP internally for
SIP signalling.

The guide is grounded in the patterns already used in this workspace:
- **F700950-gateway-mcx-lib** — a C++ core wrapped behind a boundary (JNI), driven
  by a `create → config → start → stop → destroy` lifecycle.
- **F701141-mcx-server-signalling** (`mcx_endpoint`) — a real PJSIP C endpoint
  that follows the same lifecycle (`create_mcx_endpoint` → `start_mcx_endpoint`
  → `stop_endpoint` → `destroy_mcx_endpoint`).

---

## 0. Mental model: two layers

Keep these two layers strictly separate. Almost every design decision follows
from which layer you are in.

```
            ┌─────────────────────────────────────────────┐
            │  PUBLIC C API  (the "Softil" surface)        │
            │  - opaque handles (T*)                       │
            │  - error codes (int), never exceptions       │
            │  - extern "C", POD structs only              │
            └───────────────┬─────────────────────────────┘
                            │  (cast handle <-> object)
            ┌───────────────▼─────────────────────────────┐
            │  C/C++ IMPLEMENTATION                        │
            │  - unique_ptr / shared_ptr for ownership     │
            │  - classes, templates, exceptions internally │
            │  - PJSIP endpoint, transports, call manager  │
            └──────────────────────────────────────────────┘
```

| Concern | Public C API | Internal implementation |
|---|---|---|
| Pointers | Raw `T*` (opaque handles) — forced by C | `unique_ptr`/`shared_ptr` for owning, `T*`/`T&` for observing |
| Errors | Return codes (`int status`) | Exceptions OK — **but catch them all at the boundary** |
| Types | POD structs, primitives, pointers | Full C++ |
| Strings | `const char*` + length | `std::string` |

---

## 1. Decide the object model (what is a "client/handle"?)

Before writing code, list the long-lived objects the caller will hold. In a SIP
library this is usually a small hierarchy:

```
Client / Gateway        (one per process — owns PJSIP endpoint, config, threads)
   └── Session          (one per registration / SIP account)
         └── Call        (one per active call/conversation)
```

In F700950 this is `JniMcxGatewayObject → Session → Call/Conversation`.
In the signalling server it is `mcx_endpoint → call_manager → call`.

Each of these becomes **one opaque handle type** in the public API.

---

## 2. Design the public C header (the ABI contract)

This is the only file your users see. Rules:

- One **opaque, incomplete** struct per handle type (caller can't see fields).
- Every function returns an `int` status code (except trivial getters).
- Pair every `*_create` with a `*_destroy`.
- `extern "C"` so the C++ name mangling doesn't break the ABI.
- No `std::` types, no classes, no templates in this header.

```c
/* mylib.h  --- the public C API */
#ifndef MYLIB_H
#define MYLIB_H

#include <stddef.h>   /* size_t */

#ifdef __cplusplus
extern "C" {
#endif

/* ---- error codes ---- */
typedef enum {
    MYLIB_OK            = 0,
    MYLIB_ERR_NULL      = -1,   /* a required handle/pointer was NULL     */
    MYLIB_ERR_CONFIG    = -2,   /* config file/string invalid            */
    MYLIB_ERR_SIP       = -3,   /* PJSIP init/transport failure          */
    MYLIB_ERR_STATE     = -4,   /* wrong lifecycle state (e.g. not init) */
    MYLIB_ERR_INTERNAL  = -99   /* unexpected exception caught at edge   */
} mylib_status;

/* ---- opaque handles (incomplete types) ---- */
typedef struct mylib_client  mylib_client;   /* the "gateway"/"client" */
typedef struct mylib_session mylib_session;
typedef struct mylib_call    mylib_call;

/* ---- event callbacks (how the lib talks BACK to the caller) ---- */
typedef struct {
    void (*on_registered)   (void *user_data, mylib_session *s);
    void (*on_incoming_call)(void *user_data, mylib_call *c, const char *from);
    void (*on_call_state)   (void *user_data, mylib_call *c, int new_state);
    void *user_data;        /* opaque pointer passed back to the caller */
} mylib_callbacks;

/* ---- lifecycle: create / config / start / stop / destroy ---- */

/* Option A: config from a file path */
mylib_status mylib_client_create_from_file(const char *ini_path,
                                           const mylib_callbacks *cb,
                                           mylib_client **out_client);

/* Option B: config already passed as a string buffer */
mylib_status mylib_client_create_from_string(const char *ini_text,
                                             size_t ini_len,
                                             const mylib_callbacks *cb,
                                             mylib_client **out_client);

mylib_status mylib_client_start (mylib_client *c);  /* bring up PJSIP etc. */
mylib_status mylib_client_stop  (mylib_client *c);
void         mylib_client_destroy(mylib_client *c); /* delete; NULL-safe   */

/* ---- operations ---- */
mylib_status mylib_session_register (mylib_client *c, const char *aor,
                                     mylib_session **out_session);
mylib_status mylib_call_make        (mylib_session *s, const char *dest_uri,
                                     mylib_call **out_call);
mylib_status mylib_call_hangup      (mylib_call *call);

#ifdef __cplusplus
}
#endif
#endif /* MYLIB_H */
```

Notes:
- **Out-parameters** (`mylib_client **out_client`) are the C way to return an
  object *and* a status code at the same time.
- The **callbacks struct** is how the library notifies the caller
  asynchronously (incoming call, registration done). `user_data` is an opaque
  pointer the caller gives you and you hand back untouched — never dereference
  it yourself. This mirrors `mcx_callbacks` in the signalling endpoint.

---

## 3. Step 1 of the flow — read / accept the INI config

Support **both** inputs (file path *and* in-memory string). Internally, reduce
both to the same thing: a parsed config object. This is exactly what F700950
does — `McxConfig::FromBuffer(mConfigBuffer)` parses an INI buffer regardless of
where it came from.

```cpp
// config.hpp  (internal C++)
class Config {
public:
    static Config fromFile(const std::string& path);   // reads file -> text
    static Config fromString(const std::string& text); // parses directly

    std::string  get(const std::string& key,
                     const std::string& def = "") const;
    int          getInt(const std::string& key, int def) const;
private:
    std::map<std::string, std::string> values_;
    static Config parse(const std::string& text);       // shared parser
};
```

Design point: **the file-reading and the string path converge immediately** on
the same `parse()`. Keep I/O (reading the file) separate from parsing, so the
"already a string" caller never touches the filesystem.

---

## 4. Step 2 of the flow — create the client object

The `*_create` function:
1. Reduces the config input to a `Config`.
2. Allocates the real C++ object.
3. **Does NOT** start PJSIP yet (keep create cheap and side-effect-free;
   starting the SIP stack is a separate explicit step — see step 5).
4. Catches every exception and translates it to a status code.

```cpp
// mylib.cpp  (the C API implementation)
#include "mylib.h"
#include "client_impl.hpp"
#include <memory>
#include <fstream>
#include <sstream>

/* The opaque type the C header forward-declared is defined here, hidden. */
struct mylib_client {
    std::unique_ptr<ClientImpl> impl;   // internal owner = unique_ptr
};

static std::string read_file(const char* path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open ini file");
    std::ostringstream ss; ss << f.rdbuf();
    return ss.str();
}

extern "C"
mylib_status mylib_client_create_from_file(const char* ini_path,
                                           const mylib_callbacks* cb,
                                           mylib_client** out_client) {
    if (!ini_path || !out_client) return MYLIB_ERR_NULL;
    try {
        Config cfg = Config::fromString(read_file(ini_path));
        auto handle   = std::make_unique<mylib_client>();
        handle->impl  = std::make_unique<ClientImpl>(std::move(cfg), cb);
        *out_client   = handle.release();   // hand ownership to the C caller
        return MYLIB_OK;
    } catch (const std::exception&) {
        return MYLIB_ERR_CONFIG;
    } catch (...) {
        return MYLIB_ERR_INTERNAL;          // NEVER let exceptions cross extern "C"
    }
}

extern "C"
mylib_status mylib_client_create_from_string(const char* ini_text, size_t len,
                                             const mylib_callbacks* cb,
                                             mylib_client** out_client) {
    if (!ini_text || !out_client) return MYLIB_ERR_NULL;
    try {
        Config cfg   = Config::fromString(std::string(ini_text, len));
        auto handle  = std::make_unique<mylib_client>();
        handle->impl = std::make_unique<ClientImpl>(std::move(cfg), cb);
        *out_client  = handle.release();
        return MYLIB_OK;
    } catch (const std::exception&) {
        return MYLIB_ERR_CONFIG;
    } catch (...) {
        return MYLIB_ERR_INTERNAL;
    }
}

extern "C" void mylib_client_destroy(mylib_client* c) {
    delete c;   // delete on NULL is safe; unique_ptr members clean up the rest
}
```

Key patterns:
- `make_unique` (never null, throws on OOM) builds the object; `.release()`
  transfers it to the C side as a raw pointer. `delete` in `*_destroy` takes it
  back. This is RAII *bridged* across the C boundary.
- The `struct mylib_client` wrapper holds a `unique_ptr<ClientImpl>`. The caller
  only ever sees `mylib_client*`.

---

## 5. Step 3 of the flow — start PJSIP (the SIP core)

`start()` is where the SIP stack comes up. The real `mcx_endpoint` in this
workspace follows exactly this order; the snippet below is the canonical PJSIP
bring-up sequence.

```cpp
// client_impl.cpp
pj_status_t ClientImpl::start() {
    pj_status_t st;

    // 1. Initialise the PJLIB runtime + caching pool factory.
    st = pj_init();                            if (st != PJ_SUCCESS) return st;
    pj_caching_pool_init(&cp_, &pj_pool_factory_default_policy, 0);

    // 2. Create the SIP endpoint (the "control endpoint").
    st = pjsip_endpt_create(&cp_.factory, "mylib", &endpt_);
    if (st != PJ_SUCCESS) return st;

    // 3. Add a transport (UDP/TCP/TLS) using values from the Config.
    pj_sockaddr_in addr;
    pj_sockaddr_in_init(&addr, nullptr, (pj_uint16_t)cfg_.getInt("sip-port", 5060));
    st = pjsip_udp_transport_start(endpt_, &addr, nullptr, 1, nullptr);
    if (st != PJ_SUCCESS) return st;
    // (for TLS, mirror pjsip_tls_transport_start3 as mcx_endpoint does)

    // 4. Init the transaction layer + UA layer (needed for calls/registration).
    st = pjsip_tsx_layer_init_module(endpt_);  if (st != PJ_SUCCESS) return st;
    st = pjsip_ua_init_module(endpt_, nullptr); if (st != PJ_SUCCESS) return st;

    // 5. Register YOUR module so you receive incoming SIP messages.
    //    (pjsip_mcx_usage_instance() plays this role in the signalling server.)
    st = pjsip_endpt_register_module(endpt_, &my_module_);
    if (st != PJ_SUCCESS) return st;

    // 6. Init media endpoint if you handle RTP audio.
    st = pjmedia_endpt_create(&cp_.factory, nullptr, 1, &med_endpt_);
    if (st != PJ_SUCCESS) return st;

    // 7. Spawn the worker thread(s) that run the PJSIP event loop.
    running_ = true;
    pj_thread_create(pool_, "mylib-worker", &ClientImpl::worker_thread,
                     this, 0, 0, &worker_);
    return PJ_SUCCESS;
}

/* The event loop: poll PJSIP forever until stop() flips the flag. */
int ClientImpl::worker_thread(void* arg) {
    auto* self = static_cast<ClientImpl*>(arg);
    while (self->running_) {
        pj_time_val t = { 0, 10 };      // 10 ms
        pjsip_endpt_handle_events(self->endpt_, &t);
    }
    return 0;
}
```

Wire this to the C API:

```cpp
extern "C" mylib_status mylib_client_start(mylib_client* c) {
    if (!c) return MYLIB_ERR_NULL;
    try {
        return c->impl->start() == PJ_SUCCESS ? MYLIB_OK : MYLIB_ERR_SIP;
    } catch (...) { return MYLIB_ERR_INTERNAL; }
}
```

---

## 6. Step 4 — register the SIP module (receiving messages)

PJSIP delivers incoming requests/responses to **modules** you register. Your
module's `on_rx_request` is where an incoming INVITE becomes an
`on_incoming_call` callback to your user. This is the inbound half of the API.

```cpp
static pj_bool_t on_rx_request(pjsip_rx_data* rdata) {
    if (rdata->msg_info.msg->line.req.method.id == PJSIP_INVITE_METHOD) {
        // build a mylib_call handle, then notify the user:
        //   client->callbacks.on_incoming_call(user_data, call, from);
        return PJ_TRUE;   // handled
    }
    return PJ_FALSE;      // let other modules try
}

static pjsip_module my_module_ = {
    nullptr, nullptr,
    { (char*)"mod-mylib", 9 },
    -1,
    PJSIP_MOD_PRIORITY_APPLICATION,
    nullptr, nullptr, nullptr, nullptr,
    &on_rx_request,       // <-- inbound requests land here
    nullptr, nullptr, nullptr, nullptr
};
```

---

## 7. Step 5 — operations (register, make call)

Outbound operations take a handle, validate it, and drive PJSIP. They convert
the caller's `const char*` into PJSIP's `pj_str_t`.

```cpp
extern "C" mylib_status mylib_call_make(mylib_session* s, const char* dest_uri,
                                        mylib_call** out_call) {
    if (!s || !dest_uri || !out_call) return MYLIB_ERR_NULL;
    try {
        // s->impl creates an INVITE dialog via pjsip_dlg_create_uac /
        // pjsip_inv_create_uac, sends it, wraps the result in a mylib_call.
        Call* call = s->impl->makeCall(dest_uri);
        *out_call  = reinterpret_cast<mylib_call*>(call);
        return MYLIB_OK;
    } catch (...) { return MYLIB_ERR_INTERNAL; }
}
```

---

## 8. Step 6 — stop and destroy (teardown, reverse order)

Tear down in the **reverse** order of start. Join the worker thread *before*
destroying the endpoint, or you crash polling a freed endpoint.

```cpp
void ClientImpl::stop() {
    running_ = false;                 // 1. tell the loop to exit
    if (worker_) { pj_thread_join(worker_); worker_ = nullptr; } // 2. join
    if (med_endpt_) { pjmedia_endpt_destroy(med_endpt_); med_endpt_ = nullptr; }
    if (endpt_)     { pjsip_endpt_destroy(endpt_);       endpt_ = nullptr; } // 3.
    pj_caching_pool_destroy(&cp_);    // 4.
    pj_shutdown();                    // 5.
}

ClientImpl::~ClientImpl() { if (running_) stop(); }  // RAII safety net
```

```cpp
extern "C" mylib_status mylib_client_stop(mylib_client* c) {
    if (!c) return MYLIB_ERR_NULL;
    try { c->impl->stop(); return MYLIB_OK; }
    catch (...) { return MYLIB_ERR_INTERNAL; }
}
```

---

## 9. How a caller uses the library (the whole flow end-to-end)

```c
#include "mylib.h"
#include <stdio.h>

static void on_incoming(void* ud, mylib_call* c, const char* from) {
    printf("incoming call from %s\n", from);
}

int main(void) {
    mylib_callbacks cb = {0};
    cb.on_incoming_call = on_incoming;
    cb.user_data        = NULL;

    mylib_client* client = NULL;

    /* Step 1+2: config (from file here) -> create client */
    if (mylib_client_create_from_file("server.ini", &cb, &client) != MYLIB_OK) {
        fprintf(stderr, "create failed\n");
        return 1;
    }

    /* Step 3: start PJSIP */
    if (mylib_client_start(client) != MYLIB_OK) {
        mylib_client_destroy(client);
        return 1;
    }

    /* Step 5: register + make a call */
    mylib_session* sess = NULL;
    mylib_client_... /* register, etc. */;

    /* ... run ... */

    /* Step 6: teardown, reverse order */
    mylib_client_stop(client);
    mylib_client_destroy(client);
    return 0;
}
```

Notice the caller **never** sees a C++ type, never catches an exception, and
owns each handle explicitly via create/destroy — exactly the Softil experience.

---

## 10. Ownership & null-check cheat-sheet

| Where | Pointer kind | Null check? |
|---|---|---|
| Public C API parameter (`mylib_client* c`) | raw, nullable | **Yes** — always, defensively (`MYLIB_ERR_NULL`) |
| Out-parameter (`mylib_client** out`) | raw | **Yes** |
| Internal owner of the impl | `unique_ptr` (or `shared_ptr` if shared) | No — guaranteed after `make_unique` |
| Internal "mandatory" argument | `T&` reference | No — can't be null |
| Internal "optional" argument | raw `T*` | Yes, if it may be null |
| `shared_ptr` from a lookup/factory | smart | Yes — may be empty |

Rules of thumb:
- **Raw owning pointers only at the C boundary** (forced by C); pair every
  `create` with a `destroy`.
- **Smart pointers inside** for ownership (`unique_ptr` by default, `shared_ptr`
  only when ownership is genuinely shared — e.g. the factories in F700950).
- **References/raw pointers inside** for non-owning access.
- **Catch every exception** at each `extern "C"` function; translate to a code.

---

## 11. Build notes (match this workspace)

- Produce a shared object (`.so`) just like `libmcxgateway.so` /
  `libmcx-signalling-rabbit-lib.so`.
- PJSIP static libs live under `lib/`; its headers under `include/` (as in the
  signalling project).
- Typical flags seen here: `-fPIC -DLINUX` (and `-Wno-write-strings
  -fpermissive` for the Softil-style C++ core).
- CMake `file(GLOB ...)` to collect sources, mirroring F700950's CMakeLists.

---

### Summary of the ordered steps

1. **Design two layers** — C ABI surface vs C++ implementation.
2. **Model the handles** — client → session → call (opaque types).
3. **Write the C header** — opaque handles, error codes, callbacks, `extern "C"`.
4. **Accept config both ways** — file path *and* string, converging on one parser.
5. **`create`** the client object (cheap, no side effects, exceptions caught).
6. **`start`** PJSIP — pool, endpoint, transport, tsx/ua layers, your module, media, worker thread.
7. **Register a SIP module** so inbound messages become callbacks.
8. **Expose operations** — register, make/hangup call.
9. **`stop` + `destroy`** in reverse order; join threads before freeing the endpoint.
10. Keep **raw owning pointers at the edge, smart pointers inside**, and null-check every public handle.
