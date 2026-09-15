# Softil-independent MCX client library roadmap

**Status:** Proposed implementation plan  
**Audience:** C/C++ gateway developers, Java/Akka gateway developers, MCX/FRMCS protocol specialists, QA, security, and release engineering  
**Primary integration:** `F700950-gateway-mcx-lib` inside the CeCoCo MCX gateway  
**Related Java projects:** `cecoco-gw-mcxconnector-lib`, `cecoco-mcx-sip-gateway`  
**Existing baseline:** `softil-independent-mcx-client-roadmap.md`

## 1. Objective and boundaries

The objective is to build an independent native C++ MCX client library that can replace the Softil/BEEHD implementation used by the CeCoCo MCX gateway. The new library must communicate with the target MCX Server, provide the MCX procedures required by the approved product profile, and preserve the observable Java/JNI gateway contract during migration.

The replacement is **not** a rewrite of the whole CeCoCo gateway. The initial boundary is:

```text
CeCoCo backend / Asterisk
        │ Existing SIP-facing CeCoCo gateway contract
        ▼
cecoco-gw-mcxconnector-lib / cecoco-mcx-sip-gateway
        │ Existing Java actors and JNI-facing operations
        ▼
Thin JNI compatibility adapter
        │ Neutral commands, events, identities, sessions, media
        ▼
Independent MCX client library
        │ SIP/IMS + MCX procedures + SDP + RTP/SRTP + MCData as required
        ▼
Target MCX Server
```

The initial project must **not**:

- copy Softil headers, binaries, generated code, private algorithms, or proprietary class design;
- depend on `third_parties/lib/libbeehd*` or any other Softil/BEEHD library;
- replace the existing Asterisk-facing SIP integration at the same time as the MCX client;
- assume that a generic SIP library already implements MCPTT, MCData, floor control, affiliation, functional aliases, emergency procedures, or the CeCoCo callback contract;
- implement every MCX/FRMCS feature before delivering a small, tested vertical slice.

The library should implement only the feature set approved for a particular release. Every feature must be traced to an approved requirement, protocol clause/profile decision, implementation module, and test evidence.

## 2. Verified current system

The following facts are based on the current workspace and must be kept as the starting point for the implementation plan.

### 2.1 Native gateway

Location: `C:\Git\F_CeCoCo\MCX\F700950-gateway-mcx-lib`

Current native layout:

| Existing area | Current responsibility | Replacement treatment |
|---|---|---|
| `src/jni_base` | Java-facing base classes, beans, JNI factory, method registration, thread/JVM helpers | Preserve the contract; keep or gradually simplify behind one adapter |
| `src/jni_impl` | Real JNI implementation and proxies | Convert to a thin adapter over the independent core |
| `src/jni_mock` | Mock implementation used without Softil | Promote to a deterministic contract/mock backend |
| `src/softil` | Softil-dependent identities, calls, operations, messages, media, stores | Replace incrementally; do not copy one-to-one into the new core |
| `src/common` | Shared-memory audio, frame buffers, timers, logging, utilities | Reuse only after ownership/API review; expose media and runtime ports in the new core |
| `third_parties` | Vendor SDK headers and libraries | Must not be a dependency of the independent core |

The current CMake file requires JNI, includes `src/jni_impl` and `src/softil` for the real target, and links a large vendor stack. `BUILD_MOCK=ON` selects `src/jni_mock` and a minimal link set. The new build must add a separate core target that does not require JNI, Softil headers, or vendor libraries.

### 2.2 Current native responsibilities to preserve conceptually

The existing class inventory identifies these responsibility groups:

- gateway/client lifecycle and configuration;
- Java object/factory/handle management;
- one or more MCX service identities: MCPTT, MCDATA, and MCVIDEO;
- authorization/token state and SIP registration;
- incoming/outgoing private and group calls;
- call priority, emergency indication, functional aliases, and ad-hoc criteria;
- RX/TX and PTT/floor behavior;
- SIP subscriptions, affiliation, aliases, and settings;
- MCData conversations and message operations;
- RTP/audio devices and circular/shared-memory buffers;
- asynchronous callback delivery and operation lifetime.

These are behavioral responsibilities, not a requirement to retain Softil class names or inheritance.

### 2.3 Current Java integration points

Relevant Java locations:

- `cecoco-gw-mcxconnector-lib/src/main/java/.../jni/base/JniMcxGateway.java`
- `.../JniMcxEndPoint.java`
- `.../JniMcxSession.java`
- `.../JniMcxConversation.java`
- `.../JniDemandedOpInfo.java`
- `.../JniDemandedCallInfo.java`
- `.../JniEmergencyAlertInfo.java`
- `.../JniMcxMessage*Info.java`
- `.../jni/mcx/McxGatewayImpl.java`
- `.../jni/mcx/McxEndPointImpl.java`
- `.../jni/mcx/McxSessionImpl.java`
- `.../jni/mcx/McxConversationImpl.java`
- `.../mcxconnector/McxEndpointActor.java`
- `.../mcxconnector/McxEndpointSupervisorActor.java`
- `.../mcxconnector/McxConversationActor.java`
- `cecoco-mcx-sip-gateway/src/main/java/.../session/McxSessionActor.java`
- `.../bridge/Sip2McxCallProtocolAdapter.java`
- `.../bridge/Mcx2SipCallProtocolAdapter.java`

The Java side is asynchronous. Calls such as registration, authorization, call creation, PTT, subscriptions, affiliation, aliases, settings, messaging, and emergency operations start work; native callbacks/events later update actors. The replacement must preserve asynchronous behavior, event correlation, object lifetime, and terminal event delivery.

### 2.4 Existing Java behavior that must be frozen in tests

Before implementing a real protocol stack, record and test:

- endpoint start/stop and register/unregister ordering;
- authorization-code request and acknowledgement states;
- token request/response states and preconfigured-token behavior;
- service identity values for MCPTT, MCDATA, and MCVIDEO;
- call-type numeric values and their mapping to CeCoCo communication types;
- priority mapping, including normal, imminent-peril, and emergency;
- call states and RX/TX state transitions;
- incoming/outgoing call information, aliases, calling user, criteria, and member list;
- session and conversation creation/removal timing;
- affiliation, functional-alias, settings, and emergency callbacks;
- message types and message states;
- audio device attachment/detachment and shared-memory setup;
- behavior when Java actors or endpoint objects terminate while operations are pending;
- callback thread expectations and shutdown behavior.

Do not allow the core library to depend on Java enum ordinals. Convert them in the adapter.

## 2.5 Interface-first reconstruction when implementation source is unavailable

The Softil implementation source is not available. Therefore, this project must not pretend to know Softil internals. The replacement must be derived from three observable evidence sources:

1. **Public C/C++ interface evidence**: declarations, opaque handles, function signatures, parameter structures, enum values, comments, callback signatures, documented return values, and documented lifecycle rules.
2. **Existing wrapper call-site evidence**: the order in which the current native wrapper constructs objects, sets callbacks, fills parameters, invokes operations, translates callbacks, and destroys objects.
3. **CeCoCo/MCX behavioral evidence**: Java callback consumers, configuration, sanitized SIP/SDP/RTP traces, and controlled interoperability tests against the target MCX Server.

The result is an implementation of the required **observable behavior**, not a reimplementation of hidden Softil code.

### 2.5.1 Evidence strength and limits

Use the following evidence classification in design records:

| Evidence level | Examples | What it supports |
|---|---|---|
| Strong | Public function signature; explicit callback signature; documented state/reason enum; wrapper call order; Java event consumer | API shape, input/output fields, object lifetime, observable event mapping |
| Medium | Public comments describing a procedure; configuration defaults; constants and size limits; repeated behavior in multiple call sites; sanitized wire trace | Candidate state transition, timer/size requirement, protocol field, interoperability behavior |
| Weak | Class/function name alone; unused declaration; undocumented field; one isolated trace; vendor-specific default | Investigation hypothesis only; never sufficient by itself for a requirement |
| Unknown | Internal scheduling, hidden retry policy, private encoding, licensing checks, server-side behavior not visible in traces | Must be tested, specified, or deliberately excluded |

Public headers can reveal that an operation exists and what data crosses the boundary. They cannot reliably reveal all of the following:

- internal thread scheduling and callback ordering when several events race;
- private timers and retry/backoff values not exposed in the API;
- exact SIP/XML serialization choices where multiple standards-compliant encodings exist;
- hidden validation and defaulting rules;
- behavior for malformed, duplicate, late, or out-of-order messages;
- server-specific interoperability workarounds;
- whether a feature is licensed, enabled, or compiled into a particular binary;
- codec implementation quality, packet-loss concealment, or performance;
- ownership behavior when an application destroys an object during a callback;
- undocumented security/key-management behavior.

Every unknown must be recorded as an open question with an experiment or test that can close it. Do not fill gaps by copying a Softil-looking class or guessing from a function name.

### 2.5.2 Public interface inventory already available

The available headers are enough to establish the following observable responsibility map:

| Public interface family | Observable facts | Replacement work |
|---|---|---|
| `RvV2oipClient.h` | Opaque handles for client, identity, call, conversation, message, subscription, publication, HTTP/XDM, and generic transactions; client lifecycle and size/port constants | `McxClient`, `Endpoint`, `CallSession`, `Conversation`, `OperationId`, transport/configuration limits |
| `RvV2oipClientCallbacks.h` | Client callbacks are registered after construct and before start; network-interface, TLS, SIP extensibility, XDM, and eMBMS callback families exist | `EventSink`, `NetworkMonitor`, `TlsProvider`, `SipTraceHook`, HTTP/XDM ports; add only profile-required features |
| `RvV2oipIdentity.h` | Identity status includes registration state, identity/user/server/address fields; refresh supports registrar/proxy changes; subscription, XDM, HTTP, KMC, and identity operations are exposed | `ServiceIdentity`, `RegistrationSession`, `RegistrationManager`, `SubscriptionManager`, `HttpClient`, `CredentialProvider` |
| `RvV2oipClientCall.h` | Incoming/outgoing direction; idle/offering/inviting/authenticating/proceeding/connected/dropping/disconnected/redirected/rejecting states; terminal reasons; media/security levels | `CallStateMachine`, `CallSnapshot`, `CallProcedure`, `ErrorCode`, `MediaSession`, recovery tests |
| `RvV2oipClientCallMcptt.h` | MC session handle; private, first-to-answer, ambient, video, group; normal/emergency/imminent-peril; commencement; group ID, functional alias, broadcast, user priority, ad-hoc members/criteria/location | `PrivateCallProcedure`, `GroupCallProcedure`, `PriorityPolicy`, `AdHocCallPolicy`, `EmergencyAlertProcedure` |
| `RvV2oipMPCtrlTypes.h` and `RvV2oipMbcpTypes.h` | Start/end TX/RX requests; normal/MCPTT/MCVideo call types; no-permission/pending/permission/releasing/queued states; floor request/grant/taken/deny/release/idle/revoke/queue/ack messages; timer/event names | `FloorController`, `FloorStateMachine`, `PttController`, `FloorMessageCodec`, timer and race tests |
| `RvV2oipGeneralSubs.h` | Event header, expiry, server/request URI, transport, dialog, content type/body, subscription state/reason callbacks, MC-info body and generic body-part callbacks | `SubscriptionTransaction`, `SubscriptionSnapshot`, `McxContentCodec`, bounded body handling |
| `RvV2oipPubClient.h` | PUBLISH request parameters, publication state, send/terminate/remove behavior | `PublishTransaction`, `PublishManager`, service-specific publication procedures |
| `RvV2oipConversation.h` | Conversation type includes MCData; create callback supplies identity, remote URI, MCData remote URI, and conversation ID; terminate callback; construct/destruct/getters | `Conversation`, `ConversationRegistry`, conversation lifecycle and persistence policy |
| `RvV2oipConversationMessage.h` | Message created/connecting/sending/request-received/FD-offer/answered/failed/completed states; reasons; delivery/read/file notifications; data streaming callbacks; expiration and message IDs | `MessageOperation`, `MessageStateMachine`, `MessagePayload`, streaming/backpressure tests |
| `RvV2oipGenMessage.h` | Out-of-conversation SIP MESSAGE state/reason; MC-specific callback/alert/group-selection/transfer/forwarding information families | `GenMessageProcedure`, `EmergencyAlertProcedure`, callback-request and remote-group-selection procedures |
| `RvV2oipMediaDefs.h` | Audio/video/MSRP/MBCP/MTC media types; RTP/compressed/linear formats; transmit/receive directions; G.711/G.722/AMR/EVS and other codec identifiers | `MediaPort`, `RtpSession`, `SrtpContext`, `Codec`, `MediaNegotiator`, dependency/license review |
| `RvV2oipClientCfg.h` and configuration calls | Config is injected as a buffer and values can be queried by group/identity/parameter | `ConfigParser`, `ValidatedClientConfig`, migration tool; no vendor-key dependency in core |

The header names above are evidence references for analysis. Do not copy proprietary header contents into the new library or redistribute them as part of the replacement.

### 2.5.3 Wrapper call-site evidence already available

The current wrapper shows the following observable operation sequences:

```text
ClientMcx::Init
  → RvV2oipClientConstruct
  → RvV2oipClientGetVersionInfo
  → configure callback structures
  → RvV2oipClientIdentitySetCallbacks
  → RvV2oipClientCallSetCallbacks
  → RvV2oipClientSetCallbacks

SipIdentity creation
  → RvV2oipIdentityAdd
  → RvV2oipIdentityRegister
  → registration callback
  → optional RvV2oipIdentityRefresh
  → RvV2oipIdentityUnregister
  → RvV2oipIdentityRemove

MCX authorization
  → RvV2oipIdentityMcpttBuildAuthenticateRequest
  → external authorization code
  → RvV2oipIdentityMcpttTokenRequest
  → token/authorization callback
  → register with token or publish token according to profile

Call creation
  → RvV2oipCallConstruct
  → RvV2oipCallMcInitPrivateCall or RvV2oipCallMcInitGroupCall
  → RvV2oipCallDial
  → call-state/call-notification callbacks
  → answer/reject/drop
  → media and/or MPC operations
  → disconnected callback
  → RvV2oipCallDestruct

Media
  → RvV2oipCallMediaChannelBufferImport
  → RvV2oipCallSetMediaPath
  → RvV2oipCallStartMediaExport
  → RvV2oipCallStartChannel / StopChannel
  → stop/export cleanup

Floor/PTT
  → RvV2oipCallMpcClientRequest(StartTX/EndTX)
  → MPC/MBCP state/message callback
  → RX/TX notification
  → media transmit permission changes

Subscriptions/publication
  → RvV2oipIdentityGeneralSubsConstruct
  → RvV2oipGeneralSubsSetMCConfig
  → RvV2oipGeneralSubsSubscribe/Unsubscribe
  → subscription state and body callbacks
  → RvV2oipPubClientSendPublish/SendPublishRemove

Conversation/messages
  → RvV2oipConversationConstruct
  → RvV2oipMessageConstruct
  → configure MCData payload or general message
  → send/notification/data callbacks
  → message state callback
  → message/conversation destruct
```

These sequences are not proof of the hidden implementation. They are a reliable starting point for defining equivalent neutral operations, ownership, and tests.

### 2.5.4 Interface archaeology procedure

Execute these tasks before implementing each protocol feature:

1. **Header inventory:** record every relevant declaration, enum, struct, handle, callback, return code, constant, and documented lifecycle rule.
2. **Signature catalog:** generate a table containing function, input/output parameters, ownership wording, synchronous return, asynchronous callback, and destruction function.
3. **Call-site sequence:** trace every current wrapper call from Java entry point to public API call and back to Java callback.
4. **Parameter provenance:** mark each field as Java-provided, configuration-provided, derived by the wrapper, returned by the server, or unknown.
5. **Handle graph:** map each opaque handle to its application handle, owner, parent, creation function, callback identity, terminal event, and destruction function.
6. **State matrix:** combine public enum values, callback use, Java states, and observed traces into a neutral state-transition table.
7. **Message matrix:** map each operation to SIP method, dialog/transaction, content type, MCX body, media/floor message, response, timeout, and event.
8. **Error matrix:** map negative return values and callback reasons into neutral error categories; do not expose vendor numeric values as the new API.
9. **Configuration matrix:** map each consumed configuration field to a neutral setting, default, validation rule, and secret classification.
10. **Media matrix:** map media type, format, direction, codec, buffer import/export, channel start/stop, floor state, and frame ownership.
11. **Trace experiment:** create a sanitized scripted or server trace for the positive case and at least one rejection/timeout case.
12. **Unknowns review:** list behavior still not supported by evidence and choose an explicit implementation policy: test, profile decision, conservative rejection, or out of scope.
13. **Contract test:** implement the neutral API test before the real protocol implementation.
14. **Interoperability test:** prove the behavior against the target MCX Server before marking the feature complete.

Required artifact for each feature:

```text
interface-inventory.md
call-sequence.mermaid
handle-lifetime.md
state-machine.md
message-dictionary.md
configuration-mapping.md
positive-vector
negative-vectors
contract-test
server-interoperability-result
open-unknowns.md
```

### 2.5.5 Concrete mapping from interface evidence to replacement classes

| Observed interface operation | Neutral replacement behavior | Test required |
|---|---|---|
| Construct/set callbacks/start/stop/destruct | `McxClient` lifecycle and injected `EventSink` | Start/stop idempotence, callback registration order, shutdown with pending operations |
| Opaque handle plus application handle | Strong typed ID plus registry-owned object; application context stays adapter-only | Handle lookup, invalidation, double destruction, late callback |
| Identity add/register/refresh/unregister/remove | `Endpoint`, `RegistrationSession`, `RegistrationManager` | State/retry/expiry/refresh/server restart |
| Authentication request/token request | `AuthorizationProvider`, `TokenStore`, authorization state machine | Delayed code, rejection, expiry, secure redaction |
| Call construct/dial/answer/reject/drop/destruct | `CallSession` and procedure-specific state machine | Incoming/outgoing sequences, terminal cleanup, timeout/reject mapping |
| MC private/group initialization structs | Typed `PrivateCallRequest`/`GroupCallRequest` and `CallProcedure` | Required-field validation and profile-specific serialization |
| Call-state and call-reason enums | Neutral `CallState`/`ReleaseReason` | Full transition matrix and unknown-value handling |
| MPC/MBCP request/state/message values | `FloorController` and `FloorMessageCodec` | Request/grant/deny/release/revoke/queue/race/timer tests |
| Media buffer import/path/export/channel APIs | `MediaPort` plus `RtpSession`/`Codec` | Frame ownership, direction, mute, loss, stop/release |
| General subscription and publish APIs | `SubscriptionTransaction`/`PublishTransaction` | Expiry, refresh, NOTIFY body, publication removal |
| Conversation/message creation and state callbacks | `Conversation`/`MessageOperation` state machines | IDs, expiry, notification, streaming, destruction |
| MCData/general-message callback payloads | Typed `MessagePayload` and event models | Parse/serialize, malformed content, unknown extension policy |
| Config buffer/get-parameter APIs | Neutral config schema and profile validator | Required/default/secret/invalid configuration cases |

### 2.5.6 Rules for implementation decisions from interfaces

- A declaration proves that the boundary exists; it does not prove that the feature is required for the CeCoCo product.
- A callback proves that an event is observable; it does not prove its exact ordering relative to other callbacks until tested.
- A documented terminal state should become a neutral terminal state, but the new implementation must define safe behavior for duplicate and late events.
- A parameter structure should become a typed neutral request only after field provenance and required/optional status are known.
- A public enum value must not be reused as a Java-compatible ordinal or wire value without a standards/profile decision.
- A function named `SetMCConfig`, `InitGroupCall`, or `SendPublish` must not be treated as a specification of its wire encoding. The corresponding 3GPP/FRMCS procedure and server trace decide that.
- Interface comments are valuable evidence but remain vendor documentation; use them to formulate hypotheses and verify them against approved specifications and interoperability tests.
- If no evidence exists, fail closed for security-critical or safety-critical behavior and mark the feature unsupported until clarified.
- Build the replacement from behavior and protocol requirements, not from a one-to-one class translation of the old wrapper.

### 2.5.7 New interface-driven work packages

These tasks refine WP-01, WP-02, WP-05, WP-08, WP-09, and WP-20. They are mandatory because implementation source is unavailable:

| ID | Task | Deliverable | Exit condition |
|---|---|---|---|
| IF-01 | Inventory public headers and legal access boundaries | Header inventory and IP decision | All used interface families are listed and approved for analysis |
| IF-02 | Extract signatures, handles, enums, callbacks, and constants | API catalog | Every wrapper-used declaration has ownership/return/callback notes |
| IF-03 | Trace wrapper call sequences | Sequence diagrams | Java command → wrapper → interface → callback → Java event is documented |
| IF-04 | Build handle/lifetime graph | Handle lifetime document | Every create/get/terminal/destroy path has an owner and late-callback policy |
| IF-05 | Build neutral state/error matrices | State/error tables | All supported states/reasons have explicit transitions and mappings |
| IF-06 | Build parameter/message/media matrices | Typed field and message dictionaries | Required/optional/source/validation/encoding status is known or marked unknown |
| IF-07 | Create deterministic interface-derived mock | Scripted mock scenarios | Delays, duplicates, errors, and callback races are reproducible |
| IF-08 | Capture sanitized protocol evidence | Trace/vector set | At least one positive and one negative vector exist per MVP procedure |
| IF-09 | Resolve unknowns with experiments or conservative policies | Open-unknowns decision records | No hidden assumption is silently implemented |
| IF-10 | Implement neutral API contract tests before real protocol code | Contract suite | Mock and future real backend share the same event/lifecycle assertions |
| IF-11 | Verify each procedure against the target server | Interoperability report | Wire behavior and event timeline match the approved profile |
| IF-12 | Review the interface-derived implementation for accidental vendor coupling | Dependency/API review | No proprietary type, header, binary, or unverified vendor rule leaks into core |

## 3. Definition of the target product

Before coding, approve a release capability matrix. The first release should normally be narrow:

| Capability | MVP decision | Required evidence before implementation |
|---|---|---|
| MCPTT identity and registration | Required | Target server profile, token mode, registration trace |
| MCData identity/registration | Decide separately | Product requirements and server capability |
| MCVideo | Out of MVP unless required | Product requirement and media profile |
| Private call | First call vertical slice | One incoming and outgoing trace |
| Group call | After private call | Group/floor profile and trace |
| Floor/PTT | Required for supported group procedure | MBMS/MCPTT procedure and race tests |
| RTP audio | Required for production call | Codec, SRTP, SDP, media bridge decision |
| Affiliation | Product-dependent | Required profile and operation traces |
| Functional aliases | Product-dependent | CeCoCo behavior and server procedure |
| MCData text | Product-dependent | Conversation/message profile |
| Location/status/file messages | Add separately | Exact requirement and content type |
| Emergency alert | Separate safety gate | Authorization, requirements, negative tests |
| MCVideo | Separate product decision | Codec/media/server profile |
| Offline/off-network | Explicitly out of scope unless required | Separate architecture and profile |

Use these labels for each row: `MVP`, `Phase 2`, `Later`, or `Not supported`. Do not leave scope implicit.

## 4. Work breakdown and order

The tasks below are sequential where a dependency is shown. Parallel work is allowed only when it does not bypass the preceding contract or security decision.

### WP-00 — Establish governance, scope, and ownership

**Dependencies:** None  
**Owner:** Technical lead + product owner + MCX server owner  
**Outputs:** Approved scope matrix, named reviewers, target server access, decision log

Tasks:

1. Name the first supported product profile, MCX Server version, 3GPP release, FRMCS document revisions, operating system, and deployment image.
2. Identify the first end-to-end use case: recommended initial case is one registered MCPTT identity and one private call without audio.
3. Identify the first transport, authorization mode, codec, and media path.
4. Secure a controlled MCX Server test account/profile and a method for collecting sanitized traces.
5. Define who approves protocol interpretation, security exceptions, emergency features, dependencies, and release readiness.
6. Create a decision record for every unresolved question. Never encode an assumption as a protocol rule without recording its source.

**Exit criteria:** The team can state exactly what Release 0 supports and what it does not support. A server test profile and a trace collection process exist.

### WP-01 — Freeze the Java/JNI compatibility contract

**Dependencies:** WP-00  
**Owner:** Java gateway + native/JNI maintainers  
**Primary files:** `src/jni_base`, `src/jni_impl`, `src/jni_mock`, Java `jni/base` and `jni/mcx` packages  
**Outputs:** `docs/current-jni-contract.md`, contract test suite, event timeline

Tasks:

1. Inventory every public JNI method, constructor, native handle, callback, bean field, enum, exception, and destroy path.
2. Record immediate return values versus asynchronous results.
3. Record callback order, possible duplicate callbacks, terminal events, and callbacks that may arrive during shutdown.
4. Record the ownership of `jobject`, global references, native handles, strings, arrays, and media buffers.
5. Freeze `JniDemandedOpInfo`, `JniDemandedCallInfo`, `JniEmergencyAlertInfo`, conversation, and message field mappings.
6. Freeze call type, media mode, hook mode, priority, call state, RX state, TX state, message state, and endpoint state mappings.
7. Add contract tests that run against the existing mock and the new mock backend.
8. Define an adapter-only conversion table between Java values and neutral core values.

**Exit criteria:** A new backend can be substituted behind the existing Java classes without Java source changes for the MVP, and the contract suite detects event-order or mapping regressions.

### WP-02 — Standards, FRMCS profile, and legal/IP baseline

**Dependencies:** WP-00  
**Owner:** MCX protocol specialist + legal/security reviewers  
**Outputs:** `docs/standards-baseline.md`, `docs/protocol-traceability.md`, `docs/ip-and-license-boundaries.md`

Tasks:

1. Read and record the exact revisions of the approved project documents: FIS-7970, AT-7800, FU-7120, FFFIS-7950, TOBA-7510, and any newer approved baseline.
2. Identify the applicable 3GPP Stage 1, Stage 2, and Stage 3 documents for the selected features, including MCX common architecture, MCPTT, MCData, security, IMS/SIP, and media.
3. Record the applicable IETF dependencies for SIP, SDP, RTP/RTCP, SRTP, TLS, MSRP, and related transports.
4. For every MVP feature, create a traceability row with requirement, procedure, message/content, timer, state transition, security rule, implementation module, and test vector.
5. Separate normative behavior, FRMCS profile behavior, target-server interoperability behavior, CeCoCo compatibility behavior, and unverified vendor observations.
6. Confirm that the implementation team is permitted to use the selected standards, open-source dependencies, project traces, and any existing vendor documentation.
7. Do not copy Softil source, headers, generated schemas, class names, private APIs, or proprietary behavior into the new implementation.

**Exit criteria:** Every MVP behavior has a documented source and an approved implementation/test owner. Unexplained observations remain explicitly marked as open issues.

### WP-03 — Build a dependency and third-party scorecard

**Dependencies:** WP-02  
**Owner:** Architecture + security + release engineering  
**Outputs:** `docs/dependency-matrix.md`, SBOM plan, license/CVE policy, selected versions

Evaluate candidates by capability, maturity, Linux support, event-loop integration, ABI/API stability, performance, fuzzing history, CVE process, license, static/dynamic linking terms, and ability to run without vendor code.

#### Recommended dependency categories

| Category | Candidates to evaluate | Decision rule |
|---|---|---|
| SIP/transaction/dialog | PJSIP/PJSUA, Sofia-SIP, reSIProcate, or a small in-house transaction layer over a proven parser | Must support required SIP transactions, dialogs, authentication, custom content, TLS, reliable provisional responses, UPDATE/re-INVITE, and external event-loop control |
| SDP | SIP library SDP module, PJSIP SDP, or a dedicated validated model | Must preserve media direction, payload formats, attributes, bundle/mid rules if used, and offer/answer semantics |
| RTP/RTCP | PJSIP media, dedicated RTP module, or a small project-owned adapter | Must support required packetization, sequence/timestamp handling, RTCP statistics, and controlled shutdown |
| SRTP | libsrtp2 or approved equivalent | Must match the server security profile; no insecure fallback |
| TLS/crypto | OpenSSL from the supported Linux distribution or approved pinned build | Centralize certificate validation, key loading, algorithm policy, and redaction |
| XML/content | libxml2, Expat, pugixml, or another hardened parser | Apply body-size limits, namespace validation, safe entity settings, and schema/profile checks |
| HTTP/token support | libcurl or existing approved HTTP client | Prefer opaque token handling; keep IDMS/OAuth business logic outside the core where possible |
| Event loop | Standalone Asio, libuv, libevent, or the selected SIP stack loop | One ownership model; no blocking network call on protocol/state threads |
| DNS/STUN/TURN/ICE | Server-profile-dependent resolver, libnice, or selected SIP/media stack support | Include only when required by the approved deployment profile |
| Codecs | Required approved codec libraries only | Check patent/licensing and distribution rights for AMR, AMR-WB, EVS, G.711, G.722, and any other codec |
| Audio conversion | Project-approved small resampler or media library | Add only if the production media path needs conversion |
| Test framework | GoogleTest or approved equivalent | Must support deterministic unit, parameterized, and integration tests |
| Fuzzing/sanitizers | libFuzzer/Clang, AddressSanitizer, UndefinedBehaviorSanitizer, ThreadSanitizer where practical | Required for network/content/parser and JNI boundary code |
| Trace/replay | Sanitized PCAP/libpcap tooling or project-owned text replay | Must never persist tokens, keys, credentials, or real user data |
| Java boundary | JDK/JNI headers only in the JNI target | Core library must compile without Java installed |

Do not automatically reuse the current `third_parties` directory. It contains vendor components and transitive libraries that do not constitute an independent dependency strategy.

#### Specific third-party checks

For each candidate, record:

- exact pinned version and source archive/hash;
- supported compiler and C++ standard;
- required build options;
- static versus dynamic linking behavior;
- transitive dependencies;
- license and redistribution obligations;
- CVE/advisory monitoring owner;
- ABI compatibility policy;
- thread and callback model;
- shutdown behavior;
- test coverage and fuzzing evidence;
- whether it can be removed behind a project-owned adapter.

**Exit criteria:** The core dependency set is approved, reproducible, license-compatible, security-reviewed, and represented by CMake imported targets. No vendor library is present in the core link graph.

### WP-04 — Create the independent repository/build structure

**Dependencies:** WP-01 and WP-03  
**Owner:** C++ build/release maintainer  
**Outputs:** Standalone CMake project, presets, CI build, install rules

Create a new repository or isolated directory instead of overwriting `src/softil` first.

Suggested structure:

```text
independent-mcx-client/
├── CMakeLists.txt
├── CMakePresets.json
├── cmake/
├── include/mcx/
│   ├── client.hpp
│   ├── types.hpp
│   ├── config.hpp
│   ├── commands.hpp
│   ├── events.hpp
│   ├── calls.hpp
│   ├── floor.hpp
│   ├── messaging.hpp
│   ├── media.hpp
│   ├── security.hpp
│   └── errors.hpp
├── src/
│   ├── api/
│   ├── runtime/
│   ├── identity/
│   ├── authorization/
│   ├── sip/
│   ├── sdp/
│   ├── mcx/
│   ├── calls/
│   ├── floor/
│   ├── subscriptions/
│   ├── affiliation/
│   ├── aliases/
│   ├── messaging/
│   ├── emergency/
│   ├── media/
│   ├── rtp/
│   ├── security/
│   ├── transport/
│   └── observability/
├── adapters/
│   ├── sip_backend/
│   ├── jni/
│   ├── c_api/
│   └── shared_memory_audio/
├── mock/
│   ├── mock_client/
│   ├── mock_mcx_server/
│   └── scripted_scenarios/
├── tools/
│   ├── mcx_probe/
│   ├── trace_replay/
│   ├── trace_sanitizer/
│   └── config_validator/
├── tests/
│   ├── unit/
│   ├── contract/
│   ├── state_machine/
│   ├── protocol_vectors/
│   ├── integration/
│   ├── interoperability/
│   ├── soak/
│   └── fuzz/
├── testdata/
│   ├── sip/
│   ├── sdp/
│   ├── mcx/
│   └── rtp/
└── docs/
    ├── architecture.md
    ├── current-jni-contract.md
    ├── api-contract.md
    ├── protocol-traceability.md
    ├── dependency-matrix.md
    ├── configuration.md
    ├── security.md
    └── operations.md
```

CMake targets:

```text
mcxclient-core       # no JNI, no Java, no vendor SDK
mcxclient-sip        # selected SIP/SDP/transport adapter
mcxclient-media      # RTP/SRTP/codec/media adapters
mcxclient-jni        # JNI compatibility layer only
mcxclient-c          # optional stable C ABI
mcxclient-mock       # deterministic mock backend
mcx_probe            # standalone diagnostic executable
mcx_trace_replay     # sanitized protocol replay tool
mcx_unit_tests
mcx_integration_tests
mcx_fuzz_targets
```

Required build modes:

- Debug and Release;
- warnings-as-errors for project code where feasible;
- AddressSanitizer and UndefinedBehaviorSanitizer;
- ThreadSanitizer where the selected dependencies allow it;
- code coverage;
- offline/reproducible dependency build;
- core-only build without JDK;
- JNI build with the supported JDK;
- mock build with no network/vendor dependency.

**Exit criteria:** `mcxclient-core` and its unit tests build on the target Linux/WSL toolchain without JNI, Softil, or `third_parties`.

### WP-05 — Define the neutral public API and data model

**Dependencies:** WP-01, WP-02, WP-04  
**Owner:** C++ architecture + Java integration  
**Outputs:** Public headers, API documentation, API contract tests

The public API must contain no JNI, Java, Akka, Softil, BEEHD, or vendor-specific types.

#### Core lifecycle classes

| Class/interface | Responsibility |
|---|---|
| `McxClient` | Public facade for runtime, endpoints, commands, and event delivery |
| `McxClientFactory` | Creates a client with injected dependencies |
| `ClientConfig` | Validated neutral runtime and feature configuration |
| `McxRuntime` | Owns event loop, command executor, timers, and shutdown |
| `CommandExecutor` | Serializes application commands into protocol-owned state |
| `TimerService` | Monotonic timers with cancellation and test clock support |
| `EventSink` | Receives immutable client events on a documented dispatcher |
| `OperationId`, `EndpointId`, `CallId`, `ConversationId`, `MessageId` | Strongly typed correlation identifiers |
| `Error` / `ErrorCode` | Stable categories for validation, transport, auth, protocol, media, and server errors |

#### Endpoint and identity classes

| Class/interface | Responsibility |
|---|---|
| `Endpoint` | Owns one configured logical user endpoint |
| `EndpointConfig` | Identity URI, service identities, client ID, profile, and policies |
| `ServiceIdentity` | MCPTT, MCData, or MCVideo identity and service URLs |
| `ServiceIdentityRegistry` | Stores service identities without relying on numeric enum ordinals |
| `EndpointSnapshot` | Immutable current endpoint state and service snapshots |
| `RegistrationSession` | Per-service registration state, expiry, refresh, and recovery |
| `RegistrationManager` | Coordinates registration/unregistration policy |
| `TokenStore` | In-memory token and expiry handling; never logs secrets |
| `CredentialProvider` | Injects protected credentials or token material |
| `AuthorizationProvider` | Port for external authorization-code/token flow |

#### Call and service classes

| Class/interface | Responsibility |
|---|---|
| `CallRequest` | Neutral outgoing call request |
| `CallSnapshot` | Immutable call state and call information |
| `CallSession` | Common lifecycle, signaling, media, and release ownership |
| `PrivateCallProcedure` | Private-call-specific MCX procedure |
| `GroupCallProcedure` | Group-call-specific MCX procedure |
| `BroadcastCallProcedure` | Broadcast behavior when in scope |
| `ListeningCallProcedure` | Listening behavior when in scope |
| `PreEstablishedCallProcedure` | Pre-established behavior when in scope |
| `PreArrangedCallProcedure` | Pre-arranged behavior when in scope |
| `AdHocCallPolicy` | Criteria/member/range validation and allocation |
| `CallRegistry` | Owns active call IDs and lifetime |
| `CallStateMachine` | Explicit state transitions and terminal handling |
| `PriorityPolicy` | Neutral priority and resource-priority validation/mapping |

#### Floor and media classes

| Class/interface | Responsibility |
|---|---|
| `FloorController` | Service-facing floor request/release policy |
| `FloorStateMachine` | Requesting, queued, granted, denied, revoked, and released states |
| `FloorSnapshot` | Immutable floor owner, priority, reason, and timing information |
| `PttController` | Maps application push/release commands to floor/media policy |
| `MediaPort` | Neutral frame input/output contract |
| `MediaFrame` | Bytes, length, timestamp, format, direction, and ownership metadata |
| `MediaSession` | Connects negotiated media to a port |
| `JitterBuffer` | Bounded packet/frame reordering and loss handling |
| `RtpSession` | RTP/RTCP state, packet validation, and statistics |
| `SrtpContext` | SRTP/SRTCP security adapter |
| `Codec` / `CodecFactory` | Selected codec encode/decode abstraction |
| `SharedMemoryMediaPort` | Compatibility adapter for current CeCoCo shared-memory buffers |
| `InProcessMediaPort` | Deterministic unit/integration test port |

#### MCData and service-operation classes

| Class/interface | Responsibility |
|---|---|
| `SubscriptionManager` | Generic subscription lifecycle and expiry |
| `AffiliationManager` | Affiliation/de-affiliation operations and group snapshots |
| `FunctionalAliasManager` | Alias add/remove and received alias information |
| `SettingsManager` | Settings subscription and state |
| `Conversation` | MCData conversation lifecycle |
| `ConversationRegistry` | Conversation ownership and lookup |
| `MessageOperation` | One message operation, expiry, and state |
| `MessagePayload` | Typed text, binary, status, location, file URL, or hyperlink payload |
| `EmergencyAlertProcedure` | Dedicated validated emergency operation |
| `OperationTracker` | Correlation, timeout, cancellation, duplicate, and terminal result policy |

#### Protocol and transport classes

| Class/interface | Responsibility |
|---|---|
| `SipStack` | Adapter boundary around the selected SIP implementation |
| `SipTransport` | UDP/TCP/TLS sockets and transport policy |
| `SipTransaction` | Client/server transaction correlation and timers |
| `SipDialog` | Dialog state and route/contact handling |
| `SipRegistrar` | REGISTER/unregister/refresh procedures |
| `SipAuthenticator` | Digest/AKA/profile-specific authentication boundary |
| `SipMessage` | Neutral parsed message representation |
| `SipHeaderMap` | Case-insensitive header storage with repeated-header support |
| `SipContent` | Bounded typed body representation |
| `SdpSessionDescription` | Validated SDP model |
| `SdpOfferAnswerEngine` | Offer/answer and renegotiation policy |
| `McxContentCodec` | MCX XML/body parse and serialization by selected profile |
| `McxHeaderPolicy` | Required MCX headers, priorities, identity, and content rules |
| `ProtocolTraceSink` | Sanitized structured wire/procedure trace |

**API rules:**

1. `start()` validates and starts runtime; it does not silently register identities.
2. `stop()` is idempotent, cancels work, prevents new callbacks, and waits for owned resources.
3. Every asynchronous operation has an ID, timeout, terminal outcome, and cancellation rule.
4. No callback occurs while a core lock is held.
5. Snapshots are immutable; callers never own internal protocol objects.
6. Raw SIP/XML is diagnostic data, not the primary application model.
7. Core error categories must not expose vendor error codes.

**Exit criteria:** The API compiles independently, is documented, has mock implementations, and has contract tests for lifecycle, endpoint, call, floor, media, conversation, operation, and error behavior.

### WP-06 — Implement runtime, ownership, timers, and observability

**Dependencies:** WP-05  
**Owner:** C++ core team  
**Outputs:** Runtime implementation, test clock, structured logging, metrics

Tasks:

1. Choose one threading model. Recommended initial model: one serialized protocol executor plus bounded media workers where necessary.
2. Define which thread owns each state machine and how application commands enter it.
3. Implement bounded command queues and backpressure behavior.
4. Implement monotonic timers, timer cancellation, timeout ownership, and deterministic fake time for tests.
5. Make startup, failure, stop, and destruction idempotent.
6. Add correlation IDs to commands, events, SIP transactions, dialogs, media sessions, and logs.
7. Add structured logging with central redaction for credentials, tokens, private keys, location, and user identifiers.
8. Add metrics for registrations, calls, floor requests, media packets, loss/jitter, queue depth, timers, retries, and failures.
9. Define memory/resource limits for SIP bodies, XML, SDP, messages, packet queues, and trace buffers.

**Exit criteria:** Lifecycle and failure-injection tests show no use-after-free, callback-after-stop, unbounded queue, deadlock, or timer leak.

### WP-07 — Implement the mock backend and server simulator

**Dependencies:** WP-05 and WP-06  
**Owner:** C++ test team  
**Outputs:** `MockMcxClient`, scripted server, deterministic scenarios

The mock is not merely a fake return value. It must simulate asynchronous behavior and bad behavior.

Implement scripts for:

- delayed and duplicated registration events;
- token rejection/expiry;
- incoming/outgoing calls;
- call progress, answer, reject, release, timeout, and malformed content;
- PTT request/grant/deny/revoke;
- affiliation, alias, settings, messaging, and emergency events;
- delayed, duplicate, reordered, and late callbacks;
- network disconnect and server restart;
- actor/session destruction while work is pending;
- media frames, loss, jitter, mute, and backpressure.

**Exit criteria:** The Java gateway can run through endpoint/session/conversation lifecycle using the new backend without loading Softil, and the mock can reproduce every contract-test failure deterministically.

### WP-08 — Implement SIP, transport, authentication, and dialog foundation

**Dependencies:** WP-03, WP-06, WP-07  
**Owner:** SIP/protocol team  
**Outputs:** SIP adapter, transaction/dialog tests, `mcx_probe`

Tasks:

1. Wrap the selected third-party SIP stack behind `SipStack`; do not let vendor API types leak into the domain.
2. Implement UDP, TCP, and TLS only if required by the target profile; add IPv6 if required.
3. Implement transaction/dialog correlation and route handling.
4. Implement required SIP methods: initially REGISTER, INVITE, ACK, BYE, CANCEL, response handling, and any profile-required SUBSCRIBE, NOTIFY, PUBLISH, MESSAGE, INFO, UPDATE, or re-INVITE.
5. Implement required authentication challenges and retry rules.
6. Support custom headers and repeated headers without unsafe string concatenation.
7. Validate Via, From, To, Call-ID, CSeq, Contact, Route/Record-Route, Content-Type, Content-Length, and required profile headers.
8. Implement session timers and keepalives according to the approved profile.
9. Add transport retry, DNS, connection failure, and server restart handling.
10. Build `mcx_probe` before integrating Java. It must send/receive scripted flows, print sanitized traces, and export replayable test cases.

**Exit criteria:** The probe completes a controlled SIP registration/dialog flow against a scripted peer and then the target server, with sanitized reproducible traces and deterministic transaction tests.

### WP-09 — Implement SDP and MCX content models

**Dependencies:** WP-02, WP-08  
**Owner:** Protocol team  
**Outputs:** SDP model, content codecs, message dictionaries, parser vectors

Tasks:

1. Implement a typed SDP model instead of manipulating SDP as unvalidated strings.
2. Validate media sections, direction, connection, ports, payload types, clock rates, packetization, attributes, and rejected/disabled media.
3. Implement offer/answer rules for initial INVITE, reliable provisional response, ACK, UPDATE, and re-INVITE where required.
4. Implement bounded parsing for MCX XML and other selected body types.
5. Validate namespaces, required/optional fields, enum values, URI syntax, numeric ranges, extensions, and unknown content policy.
6. Implement serializers that produce profile-compliant content but do not rely on byte-for-byte equality unless required.
7. Create a message dictionary for every supported procedure: SIP method, header, content type, body fields, mandatory/optional status, response codes, timers, and event mapping.
8. Store sanitized protocol vectors and malformed/negative vectors.

**Exit criteria:** SDP and MCX content round-trip/vector/negative tests pass, and every supported body field maps to a traceability record.

### WP-10 — Implement authorization, identities, and registration

**Dependencies:** WP-06, WP-08, WP-09  
**Owner:** Identity/security team  
**Outputs:** Identity and registration state machines, token provider, registration traces

Implement independent per-service state:

```text
NOT_CONFIGURED
  → AUTH_URL_AVAILABLE
  → CODE_REQUESTED
  → CODE_RECEIVED
  → TOKEN_REQUESTED
  → TOKEN_AVAILABLE
  → REGISTERING
  → REGISTERED
  → REFRESHING
  → RECONNECTING
  → FAILED
```

Tasks:

1. Validate endpoint and service identity configuration.
2. Implement injected `AuthorizationProvider` for the existing CeCoCo external authorization service.
3. Implement protected preconfigured-token mode for controlled deployments; make it explicit and auditable.
4. Keep IDMS/OAuth workflow outside the protocol engine unless the approved product requires the client to own it.
5. Implement token expiry, refresh, rejection, and secure memory handling.
6. Implement REGISTER, refresh, unregister, retry, keepalive, 401/403 mapping, transport failure, and server restart recovery.
7. Preserve Java-visible intermediate authorization states through the JNI adapter.
8. Do not log token contents, authorization codes, private keys, or full Authorization headers.

**Exit criteria:** A sanitized endpoint using an injected token registers, refreshes, unregisters, and recovers against the target server profile. Token and certificate validation tests pass.

### WP-11 — Implement call/session state machines without media

**Dependencies:** WP-09 and WP-10  
**Owner:** MCX call-procedure team  
**Outputs:** Call registry, procedure implementations, state/event tests

Core call states:

```text
IDLE → REQUESTED → INVITE_SENT → PROCEEDING → OFFERING
     → CONNECTED → RELEASING → DISCONNECTED
```

Incoming states:

```text
INCOMING_RECEIVED → PRESENTED → ANSWERING → CONNECTED
                  → REJECTING → DISCONNECTED
```

Tasks:

1. Implement neutral `CallRequest` and `CallSnapshot` with remote/local party, aliases, call type, direction, media mode, commencement mode, priorities, criteria, member list, and calling user.
2. Implement one outgoing private call with valid MCX SIP/SDP content and no production audio dependency.
3. Implement one incoming private call and Java-compatible session creation.
4. Implement answer, reject, cancel, hangup, remote release, timeout, busy, forbidden, unauthorized, unavailable, and media-negotiation failure mapping.
5. Correlate SIP dialog/call IDs to `CallId` and Java session handles.
6. Handle duplicate/retransmitted/late messages safely.
7. Implement cleanup when Java actors, native handles, or endpoint runtime stop unexpectedly.
8. Keep private and group behavior in separate procedure classes sharing common session infrastructure.
9. Add call information updates for aliases, functional-alias-address flags, emergency indication, and priority.

**Exit criteria:** Incoming and outgoing private calls complete without audio against the target server profile; Java receives expected state, call information, and terminal events; all failure paths are tested.

### WP-12 — Implement RTP, SRTP, codecs, and the neutral media port

**Dependencies:** WP-11, WP-03 security/codec decisions  
**Owner:** Media team  
**Outputs:** RTP/SRTP adapter, codec adapter, media tests, shared-memory adapter

Implement the media contract before connecting it to current shared memory:

```text
MediaFrame {
  payload;
  length;
  timestamp;
  sequence;
  format;
  sample_rate;
  channels;
  direction;
  ownership;
}
```

Tasks:

1. Implement `InProcessMediaPort` and generated audio tests.
2. Implement RTP packetization/depacketization, sequence/timestamp validation, SSRC handling, RTCP statistics, and bounded queues.
3. Implement SRTP/SRTCP using the approved profile and keying method. Never silently downgrade to plaintext.
4. Implement only codecs selected in the capability matrix. Verify codec licensing before distribution.
5. Implement jitter, packet loss, late packet, mute, pause/resume, and backpressure behavior.
6. Connect negotiated SDP payload types to codec instances.
7. Implement `SharedMemoryMediaPort` as an adapter around the existing `CircularBuffer`, `FrameBuffer`, `FrameBufferPool`, and `SharedMemoryManager` behavior.
8. Define frame ownership and release rules at the shared-memory boundary.
9. Test player/recorder attach/detach and cleanup on call release, network failure, and actor termination.
10. Expose media metrics without exposing raw sensitive audio in normal logs.

**Exit criteria:** Bidirectional audio works for the supported private call, through the intended production bridge, and shuts down cleanly on every tested release/failure path.

### WP-13 — Implement floor control, PTT, and group calls

**Dependencies:** WP-12 and approved group profile  
**Owner:** MCPTT procedure team  
**Outputs:** Floor state machine, group procedure, PTT tests, interoperability traces

Floor states:

```text
NO_FLOOR → REQUESTING → QUEUED → GRANTED → TALKING
                         └──────→ DENIED
GRANTED/TALKING → RELEASING → NO_FLOOR
GRANTED/TALKING → REVOKED   → NO_FLOOR
```

Tasks:

1. Implement group-call establishment separately from private calls.
2. Implement request, grant, deny, release, revoke, queue, timeout, and collision handling.
3. Couple local capture/transmit permission to floor ownership; connected media alone must not enable transmission.
4. Map floor and media behavior to Java RX/TX callbacks.
5. Preserve transmitting party, remote alias, priority, and floor reason.
6. Test simultaneous requests, priority pre-emption, stale transactions, duplicate messages, and server denial.
7. Implement late entry and group member behavior only when required by the target profile.
8. Verify whether private calls use floor procedures; do not infer this from group behavior.

**Exit criteria:** A supported group call provides deterministic floor/PTT/media behavior against the target server and under concurrent/failure tests.

### WP-14 — Add call variants one by one

**Dependencies:** WP-13  
**Owner:** MCX procedure team + product owner  
**Outputs:** Per-call-type design and test pack

Add only call types with a real approved use case and trace:

- private;
- group;
- broadcast;
- listening;
- chat;
- pre-established;
- pre-arranged;
- ad-hoc.

For each type, define before coding:

1. request URI and identity rules;
2. SIP method/dialog behavior;
3. MCX body/content;
4. offer/answer and media mode;
5. commencement/answer rules;
6. floor behavior;
7. priority/emergency behavior;
8. late-entry/release behavior;
9. CeCoCo remote-party and functional-alias mapping;
10. ad-hoc range allocation, if applicable;
11. positive, negative, duplicate, timeout, and server-restart tests.

`AdHocCallPolicy` must own range allocation and formatting. Do not put it in a SIP parser.

**Exit criteria:** Each enabled call type has a traceability row, state table, test vectors, server evidence, and Java compatibility evidence.

### WP-15 — Implement subscriptions, affiliation, aliases, and settings

**Dependencies:** WP-10 and WP-09  
**Owner:** MCX service-procedure team  
**Outputs:** Service managers, operation tracker, typed snapshots, XML/content tests

Implement in this order:

1. Generic SUBSCRIBE/NOTIFY transaction and expiry handling.
2. Affiliation subscription and affiliation/de-affiliation operations.
3. Functional-alias subscription and add/remove operations.
4. Settings subscription/request and state.
5. Incoming information parsing and immutable snapshots.

Required classes:

```text
SubscriptionManager
SubscriptionTransaction
AffiliationManager
AffiliationOperation
FunctionalAliasManager
FunctionalAliasOperation
SettingsManager
SettingsOperation
OperationTracker
GroupAffiliationSnapshot
FunctionalAliasSnapshot
SettingsSnapshot
```

Tasks:

- correlate operation IDs and terminal results;
- support duplicate NOTIFY and refresh events;
- validate XML/content and body size;
- retain raw sanitized content only for diagnostics;
- map service-specific results to Java `McxAffiliation*`, `McxAlias*`, `McxSettings*`, and token messages;
- test unregister/stop while subscriptions are active;
- distinguish MCPTT, MCDATA, and MCVIDEO service identity state.

**Exit criteria:** Required service operations are reliable across expiry, refresh, duplicate notification, server restart, and endpoint shutdown.

### WP-16 — Implement MCData conversations and messaging

**Dependencies:** WP-10, WP-09, and service scope decision  
**Owner:** MCData team  
**Outputs:** Conversation/message model, message state tests, server traces

Tasks:

1. Implement `Conversation`, `ConversationRegistry`, and conversation lifecycle.
2. Implement private/group conversation creation and termination where required.
3. Implement text messaging first.
4. Implement message IDs, pending/failure/confirmed/delivered/read/completed states as required by the profile.
5. Implement message expiry and cancellation.
6. Add binary, status, location, file URL, hyperlink, and attachment payloads only after separate requirement approval.
7. Implement the selected MCData transport/procedure; do not assume SIP `MESSAGE` is sufficient.
8. Map messages to Java `JniMcxMessage*Info` and `JniMcxMessageGroupInfo` without leaking native handles.
9. Test malformed content, oversized payloads, duplicate deliveries, out-of-order states, and conversation destruction.

**Exit criteria:** The approved message subset works in both directions with deterministic lifecycle and expiry behavior.

### WP-17 — Implement priority, emergency, and safety-critical behavior

**Dependencies:** WP-11, WP-13, WP-15, WP-16 as applicable; dedicated security approval  
**Owner:** MCX/FRMCS specialist + safety/product/security reviewers  
**Outputs:** Emergency design, authorization record, negative tests, controlled traces

Implement separately from ordinary calls:

- normal, imminent-peril, and emergency priorities;
- resource-priority mapping;
- priority changes during active sessions;
- emergency indication and alert state;
- emergency call/alert enable and cancellation;
- normal versus ad-hoc emergency information;
- criteria, group, functional alias, location, originator, and user-requested priority;
- received indicator and callback state.

Tasks:

1. Trace every emergency field to an approved requirement and protocol clause.
2. Validate authorization and allowed call/service type before sending.
3. Reject incomplete or contradictory emergency requests locally.
4. Prevent malformed input from accidentally enabling emergency behavior.
5. Add explicit audit events without logging sensitive location or identity data unnecessarily.
6. Add server-approved positive and negative scenarios.
7. Test loss of connectivity, duplicate alert, cancellation race, endpoint stop, and recovery.
8. Obtain release approval before enabling emergency features by default.

**Exit criteria:** Emergency behavior is independently reviewed, authorized, traced, tested, and demonstrated against the approved controlled server profile.

### WP-18 — Build the JNI compatibility adapter

**Dependencies:** WP-01, WP-07, and the required real-core vertical slices  
**Owner:** JNI/native integration team  
**Outputs:** `mcxclient-jni`, adapter mapping tests, Java gateway integration

Adapter classes:

| Adapter class | Responsibility |
|---|---|
| `JniGatewayAdapter` | Maps Java gateway lifecycle/configuration to `McxClient` |
| `JniEndpointAdapter` | Maps endpoint identity, authorization, registration, subscription, and service calls |
| `JniSessionAdapter` | Maps call/session requests, PTT, priority, media, answer/reject/hangup |
| `JniConversationAdapter` | Maps conversation/message operations and callbacks |
| `JniBeanConverter` | Converts Java beans to neutral requests and immutable events to Java beans |
| `JniEventDispatcher` | Attaches native callback threads safely and invokes Java callbacks |
| `JniHandleRegistry` | Owns native-to-Java handle lifetime and invalidation |
| `JniMediaAdapter` | Maps Java player/recorder calls to `MediaPort` |
| `JniErrorMapper` | Converts neutral errors to stable Java exceptions/events |

Safety requirements:

1. Only the adapter includes JNI headers.
2. Attach/detach native threads through one managed utility.
3. Hold Java global references only while the corresponding Java object is valid.
4. Never call Java while holding a protocol, registry, or media mutex.
5. Validate nulls, strings, arrays, lengths, encodings, and enum values.
6. Cancel callbacks before deleting native objects.
7. Make endpoint, session, conversation, and gateway destruction idempotent.
8. Test actor termination during signaling, media, timers, and callbacks.
9. Preserve compatibility values only at the boundary; use neutral enums internally.
10. Ensure no Softil type appears in an adapter header or public API.

**Exit criteria:** The existing Java actors run against the real independent backend for the agreed feature set and pass the same contract suite used by the mock.

### WP-19 — Integrate the current shared-memory audio path

**Dependencies:** WP-12 and WP-18  
**Owner:** Media/JNI/gateway integration team  
**Outputs:** Shared-memory ABI document, adapter tests, deployment test

Document and test:

- `setAudio(key, bufferSize, bufferCount, jitterSize, clean)` semantics;
- `CircularBuffer`, `FrameBuffer`, `FrameBufferPool`, `SharedMemory`, and manager ownership;
- producer/consumer roles;
- control buffer and write-offset behavior;
- frame format, size, timestamp, and mute/pause events;
- player and recorder device IDs;
- initialization and cleanup across process restart;
- behavior when a buffer is full, missing, stale, or inaccessible.

The core must depend on `MediaPort`, not Linux shared memory. The shared-memory implementation belongs in `adapters/shared_memory_audio` and should be replaceable by in-process/RTP/file test ports.

**Exit criteria:** The gateway’s production audio bridge works with the independent client and no shared-memory details leak into protocol/domain classes.

### WP-20 — Add conformance, interoperability, resilience, and security testing

**Dependencies:** Every enabled feature  
**Owner:** QA + protocol + security + operations  
**Outputs:** Test plan, results, sanitized evidence, release report

#### Unit tests

Cover:

- URI/identity normalization;
- config validation and secret redaction;
- SIP/SDP parsing and serialization;
- MCX content parsing/validation;
- timers/retry/backoff;
- all state-machine transitions;
- operation correlation and expiry;
- priority and emergency validation;
- RTP sequence/timestamp/jitter/loss;
- media ownership/backpressure;
- JNI conversion and handle lifetime.

#### Contract tests

Run the same suite against:

- existing Softil backend where legally and operationally available;
- current mock;
- new deterministic mock;
- new independent backend.

Compare API results, Java events, IDs, ordering, terminal behavior, demanded call information, priority mapping, media state, and shutdown behavior. Do not require the new implementation to reproduce vendor-internal timing or private APIs.

#### Protocol vectors and replay

Each scenario must include:

- software/profile versions;
- sanitized SIP messages and headers;
- SDP body;
- sanitized MCX content;
- RTP metadata or synthetic media;
- expected state/event timeline;
- expected negative behavior;
- secrets and personal data removed.

Prefer semantic assertions over byte-for-byte packet comparison.

#### Negative and fuzz tests

Fuzz SIP, SDP, XML/MCX content, RTP packets, configuration, and JNI inputs. Verify no crash, invalid transition, unbounded allocation, callback-after-destruction, secret leakage, or unsafe fallback.

#### Interoperability/soak tests

Against the controlled target server, test:

- registration, refresh, unregister, token failure, and recovery;
- incoming/outgoing private and supported group calls;
- floor/PTT races and priority;
- supported codec and SRTP combinations;
- subscriptions, affiliation, aliases, settings, messaging, and emergency features;
- network loss, server restart, certificate failure, and interface changes;
- multiple identities, simultaneous calls, long calls, and multi-day gateway soak.

Collect native structured logs, Java event traces, sanitized protocol traces, CPU/memory, queue depth, timer counts, RTP statistics, and recovery times.

**Exit criteria:** Test results meet thresholds defined in the scope and release matrix; all security, interoperability, and operational findings are closed or formally accepted.

### WP-21 — Package, deploy, observe, and roll back

**Dependencies:** WP-04, WP-18, WP-20  
**Owner:** Release/operations team  
**Outputs:** Packages, systemd/deployment changes, runbook, rollback plan

Tasks:

1. Define shared-library names, SONAME/ABI policy, install directories, JNI loading path, and runtime dependencies.
2. Build `.so` artifacts and package integration appropriate to the existing CeCoCo deployment.
3. Keep configuration separate from secrets. Use protected credential providers/files and strict permissions.
4. Add configuration versioning and migration from the current Softil-oriented INI.
5. Add health/readiness checks for runtime, endpoint registration, server reachability, and media.
6. Add structured log rotation and sensitive-data redaction.
7. Define systemd startup/shutdown ordering and graceful termination timeout.
8. Define core dump/crash diagnosis without including secrets or audio.
9. Keep old Softil packages installed but unused during pilot only if licensing and operations allow it.
10. Provide an explicit feature flag/backend selector and rollback to the old implementation.
11. Test upgrade, downgrade, interrupted deployment, certificate rotation, and configuration rollback.

**Exit criteria:** A non-production deployment can be installed, observed, upgraded, rolled back, and diagnosed using the runbook.

### WP-22 — Migrate and remove Softil

**Dependencies:** WP-20 and WP-21  
**Owner:** Product/release owner  
**Outputs:** Pilot report, migration sign-off, removal plan

Migration order:

1. Keep Java actors and existing backend-facing behavior unchanged.
2. Run all Java contract tests against the new mock.
3. Run the independent core against scripted protocol peers.
4. Run token-based registration against the target server.
5. Enable one private-call no-audio path.
6. Enable media and compare event/media timelines.
7. Enable group/floor and other approved services one at a time.
8. Run old and new implementations in controlled comparable environments.
9. Pilot with feature flag and rollback available.
10. Obtain product, protocol, security, licensing, interoperability, and operations sign-off.
11. Remove Softil headers, link directories, binaries, configuration keys, packaging dependencies, and dead compatibility code only after the sign-off.

**Exit criteria:** The agreed feature set operates without Softil runtime or build dependencies, and rollback/removal evidence is complete.

## 5. Detailed architecture

### 5.1 Layering rule

```text
Public neutral API / optional C ABI
        │
Runtime, command queue, timers, lifecycle
        │
Endpoint, authorization, registration
        │
MCX procedures and domain state machines
        │
SIP transactions/dialogs + SDP + MCX content
        │
RTP/SRTP + codecs + media ports
        │
Transport, TLS, DNS, logging, metrics
        │
JNI/shared-memory/third-party adapters
```

Rules:

- Domain code must not include JNI, Java, Akka, or third-party headers.
- Protocol code must not know about Asterisk or `CallCommand`.
- JNI code must not decide MCX business behavior.
- Network callbacks must be converted into owned immutable events before crossing the core boundary.
- State machines must own their timers and cancel them on terminal state.
- One registry must own each resource type; no parallel hidden stores.

### 5.2 Recommended ownership model

```text
McxClient
 ├── McxRuntime
 ├── DependencyContainer
 ├── EndpointRegistry
 │    └── Endpoint
 │         ├── ServiceIdentityRegistry
 │         ├── RegistrationManager
 │         ├── SubscriptionManager
 │         ├── AffiliationManager
 │         ├── FunctionalAliasManager
 │         ├── SettingsManager
 │         ├── CallRegistry
 │         │    └── CallSession
 │         │         ├── CallProcedure
 │         │         ├── FloorController
 │         │         ├── MediaSession
 │         │         └── SipDialog
 │         └── ConversationRegistry
 │              └── Conversation
 │                   └── MessageOperation
 └── EventSink / Observability
```

Use IDs and snapshots across boundaries. Avoid exposing `shared_ptr` ownership to Java or callers unless the lifecycle contract explicitly requires it.

### 5.3 State-machine minimum set

The implementation must contain explicit transition tables and tests for:

- client runtime;
- endpoint lifecycle;
- per-service authorization/token;
- per-service registration;
- SIP transaction/dialog;
- private call;
- group call;
- floor/PTT;
- media;
- subscriptions;
- affiliation;
- functional aliases;
- settings;
- conversations/messages;
- emergency alert;
- network recovery.

Every state table must specify allowed commands, outgoing messages, timers, event(s), failure result, retry behavior, duplicate behavior, and terminal cleanup.

## 6. Configuration design

Do not expose vendor-specific INI keys as the new public configuration. Define a versioned neutral schema:

```text
runtime
  worker model
  queue limits
  shutdown timeout
  body/packet limits

endpoint
  endpoint ID
  user identity URI
  display name
  client ID
  enabled service identities

server
  registrar URI
  proxy URI
  MCPTT/MCData/MCVideo service URIs
  transport
  local signaling address/port
  DNS/route policy

authorization
  provider mode
  token provider reference
  authorization URL handling
  refresh policy

registration
  expiry
  retry/backoff
  keepalive
  timeout

media
  RTP port range
  codec allow-list
  sample rate/channels
  packetization
  jitter limits
  SRTP profile

features
  enabled call types
  floor/PTT
  affiliation
  functional aliases
  settings
  messaging
  emergency
  MCVideo

compatibility
  Java/JNI contract version
  shared-memory layout/version
  legacy enum mappings
  legacy call ID policy

security
  CA reference
  client certificate/key reference
  hostname verification
  allowed TLS algorithms
  token redaction policy

observability
  log level
  metric endpoint
  sanitized trace mode
```

Configuration tasks:

1. Define schema version and defaults.
2. Validate cross-field constraints before startup.
3. Separate ordinary configuration from secrets and key material.
4. Redact secret values in errors, dumps, traces, and metrics labels.
5. Support profile-specific capability validation.
6. Provide a migration document from current `rvbeehd*.ini` values to neutral values.
7. Do not promise a one-to-one mapping for vendor-only options.

## 7. Security requirements

The library handles identities, tokens, certificates, private keys, user identifiers, location, emergency information, and media. Security is a first-class workstream.

Required controls:

- TLS certificate-chain and hostname validation enabled by default;
- no “accept all certificates” production option;
- supported TLS version/cipher policy documented and pinned by profile;
- keys loaded from protected files/provider, never ordinary logs or command-line arguments;
- tokens and authorization codes kept out of traces and crash reports;
- secure deletion/short lifetime where feasible for credential buffers;
- bounded SIP/XML/SDP/message allocations;
- XML parser hardening and namespace validation;
- authentication and authorization failures distinguishable without leaking sensitive detail;
- sensitive URI/location/identity redaction in diagnostics;
- dependency CVE scanning and update ownership;
- fuzzing of all network/content/JNI inputs;
- audit record for emergency and security-sensitive actions;
- no debug packet capture in production without explicit sanitized mode and retention policy.

Security exit criteria must be separate from functional “call works” criteria.

## 8. Traceability and documentation deliverables

The project is not complete when classes compile. Maintain these documents under change control:

| Document | Content |
|---|---|
| `docs/scope-matrix.md` | Supported services, call types, codecs, transports, non-goals |
| `docs/current-jni-contract.md` | Java methods, callbacks, states, ownership, event order |
| `docs/standards-baseline.md` | Exact standards/profile editions and applicability |
| `docs/protocol-traceability.md` | Requirement → procedure → message → module → test |
| `docs/architecture.md` | Layers, ownership, threads, state machines, adapters |
| `docs/api-contract.md` | Public API, IDs, snapshots, errors, event ordering |
| `docs/dependency-matrix.md` | Versions, capabilities, licenses, CVEs, build options |
| `docs/configuration.md` | Neutral schema, validation, secrets, migration |
| `docs/security.md` | Threats, trust boundaries, credential/certificate policy |
| `docs/test-plan.md` | Unit, contract, vectors, fuzz, interop, soak, acceptance |
| `docs/operations.md` | Packaging, health, logs, metrics, recovery, rollback |
| `docs/decision-records/` | Unresolved interpretations and approved design decisions |

For each feature, record:

```text
Feature/use case
Product requirement and revision
3GPP/FRMCS clause and release
SIP/SDP/RTP/security dependency
Target server capability/profile
Neutral API command/event
State-machine transitions and timers
Implementation module/classes
Positive vector/trace
Negative/failure test
CeCoCo/JNI compatibility requirement
Open issue/deviation
Owner/reviewer/status
```

## 9. Recommended milestone sequence

### Milestone M0 — Evidence and scope

Deliverables: WP-00, WP-01, WP-02, WP-03.  
Success: first use case, protocol baseline, JNI contract, server access, dependency/IP decisions approved.

### Milestone M1 — Independent core and mock

Deliverables: WP-04, WP-05, WP-06, WP-07.  
Success: core builds without vendor/JNI; Java gateway lifecycle works against new mock.

### Milestone M2 — Registration probe

Deliverables: WP-08, WP-09, WP-10.  
Success: standalone probe and core register/refresh/unregister with injected token against target profile.

### Milestone M3 — Private call signaling

Deliverables: WP-11 and WP-18.  
Success: one outgoing and incoming private call without production audio, with compatible Java events and clean release.

### Milestone M4 — Production audio

Deliverables: WP-12 and WP-19.  
Success: supported codec/RTP/SRTP path and shared-memory bridge work bidirectionally.

### Milestone M5 — Group/PTT

Deliverables: WP-13 and first approved WP-14 variant.  
Success: group call and floor behavior pass concurrency, failure, and interoperability tests.

### Milestone M6 — Product services

Deliverables: WP-15, WP-16, WP-17 as approved.  
Success: each enabled service has independent evidence and Java compatibility.

### Milestone M7 — Production pilot

Deliverables: WP-20, WP-21, WP-22.  
Success: hardened package, observability, rollback, pilot, and removal decision.

## 10. Definition of done for replacing Softil

The replacement is ready only for the explicitly approved feature set when:

- the independent core builds without Softil/BEEHD headers, binaries, libraries, or runtime loading;
- the core builds without JNI or a JDK;
- JNI is a thin, tested compatibility adapter;
- the supported capability matrix is explicit;
- standards, FRMCS, server-profile, and CeCoCo requirements are traced;
- registration, calls, floor, media, service operations, messaging, emergency, and recovery state machines have transition tests for enabled features;
- malformed, duplicate, reordered, late, and oversized input is safe;
- Java callback order, IDs, state mappings, and lifecycle are compatible;
- production media works through the intended bridge;
- security tests, dependency/license review, sanitizers, fuzzing, and soak testing pass;
- interoperability is demonstrated against the target MCX Server profile;
- operations has health checks, metrics, logging, packaging, upgrade, rollback, and incident procedures;
- no credentials, tokens, private keys, real user data, or unapproved traces are committed;
- product, protocol, security, legal/IP, and operations owners sign off.

## 11. First 10 implementation tasks

Start here; do not begin by rewriting the entire `src/softil` directory.

1. Freeze `docs/current-jni-contract.md` from current Java/JNI classes and add contract tests.
2. Create the independent CMake project and prove a core-only build with no JDK or vendor paths.
3. Define `EndpointSnapshot`, `CallRequest`, `CallSnapshot`, `Event`, `MediaFrame`, `Error`, and typed IDs.
4. Implement runtime, command serialization, timers, shutdown, logging, and redaction.
5. Implement the deterministic mock and connect the current Java gateway to it.
6. Create `docs/scope-matrix.md` and `docs/standards-baseline.md` for the first server/profile/use case.
7. Build `mcx_probe` and the sanitized trace/replay workflow.
8. Select and pin SIP/SDP/RTP/SRTP/TLS/XML dependencies through the scorecard.
9. Implement injected-token registration and recovery.
10. Implement one outgoing and incoming private call without audio, then add media.

The first meaningful success target is:

```text
Independent core builds without Softil
        +
Java gateway runs against the independent mock
        +
Standalone probe registers with the target server
        +
Independent client completes one private call without audio
        +
The call releases cleanly and all expected Java events arrive
```

That target proves the architecture, build boundary, protocol boundary, lifecycle, and migration strategy before the project takes on floor control, full messaging, emergency behavior, and every MCX variant.



## 12. Exact implementation order against an existing MCX Server

This section is the operative order for implementation. Follow it instead of implementing classes in the order in which they appear in the old Softil wrapper.

### 12.1 The direct answer: what to implement first

Implement the following first:

```text
1. MCX Server profile and test-identity verification
2. Independent core runtime + deterministic mock
3. SIP/SDP/TLS transport foundation
4. MCPTT service identity authorization and registration
5. One private MCPTT call, signaling only, without production audio
6. RTP/SRTP audio for that private call
7. Minimal affiliation only if the server requires it
8. Group MCPTT call establishment
9. MBCP/MPC floor control and PTT
10. Priority and emergency behavior
11. Functional aliases and settings
12. MCData conversations and messaging
13. Remaining call variants and optional services
14. Recovery, soak, packaging, and Softil removal
```

The first actual MCX feature is therefore **MCPTT identity registration**. The first actual call feature is **a private MCPTT call**. Do not begin with group calls, MCData, emergency, MCVideo, or all codecs.

A generic MCPTT service implementation is too broad as a first task. Break it into this vertical slice:

```text
MCPTT identity
  → authorization/token
  → SIP registration
  → private call signaling
  → private call media
```

Only after this works against your server should you add:

```text
group call
  → floor/PTT
  → affiliation/aliases/settings
  → priority/emergency
  → MCData
```

### 12.2 Phase P0 — Interrogate and freeze the existing MCX Server profile

**Purpose:** Use the server that is already available to remove assumptions before writing protocol procedures.  
**Do not implement yet:** MCX call logic, floor logic, emergency, or MCData.  
**Dependencies:** None.

#### Actions

1. Create `docs/server-profile.md`.
2. Record the server software/version and the enabled MCX services.
3. Record the MCPTT service URI, registrar URI, proxy URI, authorization URL, token URL or token-injection method, and any required HTTP/XDM/KMC endpoints.
4. Record the client identity format, user identity, service identity, client ID, domain, and test group/private-user identities.
5. Record the signaling transport: UDP, TCP, TLS, IPv4/IPv6, local bind address, server address, and ports.
6. Record the certificate chain, hostname/SAN policy, client certificate requirement, and test certificate installation path.
7. Record registration expiry, keepalive, session timer, retry, and server timeout values from the actual deployment profile.
8. Record the server-supported codecs and exact SDP payload formats. Start with one audio codec only.
9. Determine whether a private MCPTT call can be made without explicit group affiliation.
10. Determine whether a group call requires prior affiliation, a preconfigured group, a subscription, or a separate service operation.
11. Determine whether the server uses MBCP over RTP/RTCP and whether it requires a separate media-control stream.
12. Determine whether the server requires token-in-REGISTER, token publication, or another profile-specific authorization procedure.
13. Create a test matrix with positive and negative server cases.
14. If the old Softil binary can connect to the server, run it only as a black-box interoperability reference and capture sanitized SIP/SDP/RTP/MBCP traces. Do not inspect, copy, or depend on its implementation.

#### Server tests

Perform these tests before coding protocol behavior:

| Test | Expected evidence |
|---|---|
| TCP/TLS connectivity | Successful connection and certificate validation |
| Invalid certificate | Client rejects it; no insecure fallback |
| Invalid identity/token | Server rejects with known status/reason |
| Valid registration | REGISTER response, expiry, service identity |
| Registration refresh | Refresh sequence and timing |
| Unregister | Clean removal and final state |
| Private-call capability | Server accepts or clearly rejects a private-call attempt |
| Group capability | Server capability/profile information or controlled group test |
| Codec capability | Accepted SDP offer/answer for selected codec |
| Floor capability | Known MBCP/MPC behavior, if testable |

#### Deliverables

- `docs/server-profile.md`
- `docs/server-capability-matrix.md`
- sanitized registration trace;
- sanitized private-call capability/trace if available;
- certificate and token-handling procedure;
- list of server-specific unknowns;
- one selected first codec and transport;
- one selected test user and one selected private destination.

#### Exit criteria

You can answer these questions with evidence:

```text
Which MCPTT identity is registered?
Which URI receives REGISTER?
Which transport and TLS policy are used?
How is the user authorized?
Which token path is used?
Which private user is the first destination?
Which codec is accepted?
Can the first private call be attempted without affiliation?
What exact server response proves registration?
```

If any answer is unknown, remain in P0.

### 12.3 Phase P1 — Create the independent core and the contract/mock backend

**Purpose:** Prove object lifetime and the Java contract without network uncertainty.  
**Dependencies:** P0 and WP-01.

#### Implement first

Core classes:

```text
McxClient
McxClientFactory
ClientConfig
McxRuntime
CommandExecutor
TimerService
EventSink
Error / ErrorCode
EndpointId / CallId / OperationId
Endpoint
EndpointConfig
EndpointSnapshot
CallRequest
CallSnapshot
MediaFrame
MediaPort
```

Mock classes:

```text
MockMcxClient
MockEndpoint
MockRegistrationScenario
MockPrivateCallScenario
MockMediaPort
ScriptedEventSink
FakeClock
```

#### Actions

1. Create a core-only CMake target with no JNI and no `third_parties` include/link path.
2. Define strong typed IDs and immutable snapshots.
3. Define start/stop behavior and command rejection after stop.
4. Implement the serialized command executor and fake timer service.
5. Implement endpoint creation/removal.
6. Implement mock registration events: `REGISTERING`, `REGISTERED`, `UNREGISTERING`, `UNREGISTERED`, and failure.
7. Implement mock outgoing/incoming private-call lifecycle.
8. Implement mock answer/reject/hangup and late-event behavior.
9. Implement a test media port with generated frames.
10. Implement the JNI adapter against the mock, not the real network client.
11. Run the Java gateway using the new mock and compare events with the existing mock.

#### Tests

- start/stop twice;
- endpoint start/stop twice;
- call created and removed exactly once;
- duplicate registration event;
- callback after endpoint stop;
- Java actor termination during active call;
- answer/reject/hangup race;
- media port attach/detach while call is stopping;
- invalid IDs and commands in invalid states.

#### Exit criteria

The Java gateway can create an endpoint, observe registration, create a session, observe call events, attach media, terminate the session, and stop the gateway with no Softil library loaded.

Do not proceed to real MCX protocol code until this passes.

### 12.4 Phase P2 — Implement the SIP/SDP/TLS foundation and probe

**Purpose:** Make network behavior observable before adding MCX semantics.  
**Dependencies:** P0, P1, WP-03.

#### Implement classes

```text
SipStack
SipTransport
SipMessage
SipHeaderMap
SipTransaction
SipDialog
SipRegistrar
SipAuthenticator
SdpSessionDescription
SdpMediaDescription
SdpOfferAnswerEngine
TlsContext
ProtocolTraceSink
McxProbe
```

#### Actions

1. Select and pin the SIP stack according to the dependency scorecard.
2. Wrap it in project-owned interfaces so no third-party type reaches the MCX domain layer.
3. Implement UDP/TCP/TLS support needed by `server-profile.md`.
4. Implement SIP message parsing and serialization with bounded header/body sizes.
5. Implement transaction and dialog correlation.
6. Implement Digest/AKA/profile-specific authentication boundary as required by the server.
7. Implement SDP parsing and serialization for the selected audio codec.
8. Implement offer/answer validation, direction, port, payload, clock rate, and packetization checks.
9. Build `mcx_probe` before connecting it to Java.
10. Add a sanitized wire logger and replay format.

#### Server tests

1. Run the probe against a scripted SIP peer.
2. Run the probe against the server transport endpoint.
3. Verify TLS certificate rejection with an invalid certificate.
4. Verify SIP authentication challenge handling.
5. Verify the probe can send and receive a harmless registration transaction.
6. Compare the probe’s message semantics with the known-good server/Softil trace if available; do not require identical header ordering.

#### Exit criteria

The probe can connect to the MCX Server using the selected transport, parse/serialize SIP and SDP, handle authentication challenges, and generate a sanitized trace. The core can run without Java.

### 12.5 Phase P3 — Implement MCPTT identity authorization and registration

**Purpose:** Establish the first real MCX service connection.  
**Dependencies:** P2.

This is the first real MCX implementation phase.

#### Implement classes

```text
ServiceIdentity
McpttIdentity
AuthorizationProvider
AuthorizationRequest
AuthorizationStateMachine
TokenStore
RegistrationSession
RegistrationManager
SipRegistrarAdapter
RegistrationEventTranslator
```

#### Implement in this order

1. `ServiceIdentity` with MCPTT only. Do not add MCDATA/MCVIDEO yet.
2. Endpoint configuration validation for the server profile.
3. Credential/token provider interface.
4. Preconfigured-token path for controlled testing if supported by the server.
5. External authorization-code path if the server requires it.
6. Token expiry and refresh representation.
7. SIP REGISTER construction using the selected server profile.
8. Authentication challenge retry.
9. Registration success/failure mapping.
10. Registration refresh.
11. Explicit unregister.
12. Transport/server restart recovery.
13. Java endpoint event mapping.

#### Registration states

```text
NOT_CONFIGURED
  → AUTHORIZATION_REQUIRED
  → AUTHORIZATION_CODE_REQUESTED
  → TOKEN_REQUESTED
  → TOKEN_AVAILABLE
  → REGISTERING
  → REGISTERED
  → REFRESHING
  → UNREGISTERING
  → UNREGISTERED
  → FAILED
  → RECONNECTING
```

The internal state can be more precise than the Java enum. The JNI adapter must preserve the Java-visible states.

#### Server tests

1. Valid MCPTT identity with valid token.
2. Invalid token.
3. Expired token.
4. Delayed authorization response.
5. Registration refresh.
6. Unregister and re-register.
7. Server restart while registered.
8. Network disconnect while registered.
9. Certificate failure.
10. Wrong MCPTT service URI.

Capture for each test:

```text
request/response SIP messages
status/reason
registration expiry
callback/event timeline
retry timing
final neutral state
```

#### Exit criteria

The independent client registers the MCPTT identity with the existing server, refreshes it, unregisters it, and recovers from a controlled disconnect. Java sees the expected authorization and registration events.

Do not implement private calls until registration is stable.

### 12.6 Phase P4 — Implement one private MCPTT call without media

**Purpose:** Prove the first MCX call procedure while excluding RTP/codec complexity.  
**Dependencies:** P3.

This is the first call to implement. Do not start with group calls.

#### Implement classes

```text
PrivateCallRequest
PrivateCallProcedure
CallSession
CallStateMachine
CallRegistry
CallInfo
CallReleaseReason
McpttInfoCodec
PrivateCallSdpPolicy
CallEventTranslator
```

#### Outgoing call actions

1. Accept a neutral `CallRequest` from the JNI adapter.
2. Validate that the endpoint is `REGISTERED` for MCPTT.
3. Validate destination URI and private-call type.
4. Apply normal priority only initially.
5. Apply the selected commencement/hook mode from the server profile.
6. Build the private-call MCPTT information body required by the profile.
7. Build an SDP offer with one selected audio stream, even if the media engine is initially disabled.
8. Create the SIP dialog and send INVITE.
9. Handle provisional responses and authentication if challenged.
10. Handle final success and ACK.
11. Emit connected state only after the required signaling condition is met.
12. Support local hangup with BYE/cancel according to the current state.
13. Handle remote release.
14. Map failure responses to neutral reasons.
15. Destruct the call only after terminal state and callback delivery policy are satisfied.

#### Incoming call actions

1. Detect incoming INVITE for the registered MCPTT identity.
2. Parse and validate SDP and MCX content.
3. Extract remote party, local party, aliases, calling user, priority, and call type.
4. Create `CallSession` and emit an incoming-call event.
5. Support answer and reject.
6. Send the appropriate final response/ACK/BYE sequence.
7. Handle remote cancel before answer.
8. Destruct after the terminal event.

#### Initially exclude

- group calls;
- floor/PTT;
- emergency priority;
- broadcast/listening/ad-hoc variants;
- functional-alias addressing unless required for the first private test;
- MCData;
- multiple codecs;
- media transmission.

#### Server tests

- outgoing private call accepted;
- outgoing private call rejected/busy/forbidden;
- outgoing timeout;
- local cancel before answer;
- remote release after connection;
- incoming private call answer;
- incoming private call reject;
- remote cancel before answer;
- malformed/unsupported MCX content;
- duplicate/late SIP response.

#### Exit criteria

One outgoing and one incoming private MCPTT call complete signaling-only against the real server. The Java gateway receives correct session creation, state, demanded call information, and terminal events. No audio is required yet, but SDP must be valid.

### 12.7 Phase P5 — Add audio to the private MCPTT call

**Purpose:** Make the first complete call useful while keeping call topology simple.  
**Dependencies:** P4.

#### Implement classes

```text
MediaNegotiator
RtpSession
RtcpSession
SrtpContext
CodecFactory
SelectedAudioCodec
JitterBuffer
MediaSession
InProcessMediaPort
SharedMemoryMediaPort
```

#### Implement in this order

1. Use `InProcessMediaPort` with generated test frames.
2. Implement the one codec selected in `server-profile.md`.
3. Implement RTP send/receive and timestamp/sequence checks.
4. Implement SRTP/SRTCP if required by the server profile.
5. Validate negotiated payload type and direction.
6. Implement receiver jitter buffering and packet-loss metrics.
7. Connect media to `CallSession` after signaling reaches the correct state.
8. Couple local transmit permission to the private-call policy; do not assume connected means transmitting.
9. Add mute/pause/resume and media failure states.
10. Implement the shared-memory adapter around the current CeCoCo buffer contract.
11. Test player/recorder attach and release.

#### Server tests

- bidirectional audio;
- receive-only and transmit-only direction;
- SRTP key/crypto failure;
- unsupported codec;
- no RTP timeout;
- packet loss/jitter;
- call release while packets are queued;
- Java actor stop while media is active;
- shared-memory buffer full/empty/recreated.

#### Exit criteria

The first private MCPTT call has bidirectional audio through the intended gateway media path, and all signaling/media resources are released on normal and abnormal termination.

### 12.8 Phase P6 — Implement only the affiliation needed by the server

**Purpose:** Remove the minimum group-call prerequisite without implementing every service operation.  
**Dependencies:** P3; execute before P7 only if the server requires affiliation.

Do not implement affiliation merely because the old wrapper has an affiliation class. First prove from the server profile whether group calls require it.

#### Implement classes

```text
AffiliationManager
AffiliationTransaction
AffiliationSnapshot
GroupIdValidator
OperationTracker
McxSubscriptionTransaction
McxInfoCodec
```

#### Actions

1. Implement MCPTT affiliation service configuration.
2. Implement required SUBSCRIBE/NOTIFY transport.
3. Implement affiliation request for one test group.
4. Correlate operation ID and group ID.
5. Parse the resulting group affiliation information.
6. Expose an immutable affiliation snapshot.
7. Implement de-affiliation.
8. Implement expiry/refresh and server restart handling.
9. Map operation results to Java affiliation events.

#### Server tests

- affiliate one valid group;
- invalid/not-authorized group;
- duplicate affiliation request;
- de-affiliation;
- subscription refresh;
- NOTIFY with changed group state;
- server restart while affiliated.

#### Exit criteria

The client can establish and observe the minimum affiliation state needed for the server’s group-call test. If the server allows group calls without affiliation, record that result and defer this phase until required.

### 12.9 Phase P7 — Implement group MCPTT call establishment without PTT

**Purpose:** Establish group session signaling before adding floor races.  
**Dependencies:** P5 and P6 if required.

#### Implement classes

```text
GroupCallRequest
GroupCallProcedure
GroupCallConfig
GroupCallInfo
GroupIdentityPolicy
GroupSdpPolicy
GroupCallEventTranslator
```

#### Implement in this order

1. Validate group URI and configured/affiliated group policy.
2. Support one group call type only: the server’s first accepted type, normally chat or prearranged.
3. Support normal priority only.
4. Support one audio codec and the already working RTP/SRTP path.
5. Build the group MCPTT information body.
6. Build and send group INVITE.
7. Handle incoming group INVITE if required by the product.
8. Establish the dialog and media session.
9. Emit group session state before floor control is enabled.
10. Release the group session cleanly.

#### Server tests

- outgoing call to one valid group;
- invalid group;
- unauthorized group;
- incoming group call if supported;
- group release;
- server rejects unsupported group type;
- group call after affiliation;
- group media establishment.

#### Exit criteria

A normal-priority group call can be established and released with stable signaling and media. PTT remains disabled or blocked until P8 is complete.

### 12.10 Phase P8 — Implement MBCP/MPC floor control and PTT

**Purpose:** Add the mission-critical transmit-control behavior.  
**Dependencies:** P7.

#### Implement classes

```text
FloorController
FloorStateMachine
FloorMessageCodec
MpcTransport
MbcpSession
PttController
FloorSnapshot
TransmittingParty
FloorTimerSet
```

#### Implement in this order

1. Start with local `requestFloor()` and `releaseFloor()` commands.
2. Implement `NO_FLOOR` and `REQUESTING`.
3. Parse server `FLOOR_GRANTED` and move to `GRANTED`.
4. Start audio transmit only after the profile-required grant condition.
5. Map grant to Java TX state.
6. Implement local release and stop transmit.
7. Parse `FLOOR_DENY` and map the reason.
8. Parse `FLOOR_REVOKE` and immediately stop transmit.
9. Parse `FLOOR_IDLE`/`FLOOR_TAKEN` and map remote transmitter/RX state.
10. Add queue-position messages only if the server uses them.
11. Add floor timers from the approved profile, not guessed values.
12. Add duplicate, stale, simultaneous, and priority-race handling.
13. Add emergency/imminent-peril floor behavior only in P9.

#### Server tests

- floor request granted;
- floor request denied;
- floor release;
- server revoke while transmitting;
- another user takes floor;
- simultaneous requests;
- queue position;
- request timeout;
- duplicate floor message;
- call release during floor request;
- media blocked without floor;
- transmit stopped immediately on revoke.

#### Exit criteria

PTT is deterministic against the server: the client transmits only while authorized, stops on release/revoke/disconnect, reports RX/TX correctly, and survives duplicate/late floor messages.

### 12.11 Phase P9 — Add priority and emergency behavior

**Purpose:** Add safety-critical behavior only after ordinary signaling, media, and floor behavior are stable.  
**Dependencies:** P4, P7, P8; dedicated product/security approval.

#### Implement classes

```text
PriorityPolicy
ResourcePriorityMapper
EmergencyCallPolicy
EmergencyAlertProcedure
EmergencyAlertValidator
EmergencyEventTranslator
EmergencyAuditSink
```

#### Implement in this order

1. Add normal versus emergency/imminent-peril as typed neutral values.
2. Validate allowed priority changes for each call type.
3. Serialize resource-priority and alert-indication fields according to the selected profile.
4. Test priority change on an active private call if required.
5. Test priority change on an active group call if required.
6. Implement emergency call indication.
7. Implement emergency alert send/cancel.
8. Implement received emergency alert parsing.
9. Implement location/criteria/functional-alias fields only when required.
10. Add audit and redaction behavior.

#### Server tests

- normal call;
- emergency private call if supported;
- emergency group call if supported;
- imminent-peril group call if supported;
- invalid priority change;
- emergency alert send/cancel;
- duplicate emergency alert;
- malformed emergency content;
- unauthorized emergency request;
- emergency operation during network loss.

#### Exit criteria

Emergency behavior is approved, traceable, validated, tested against the server, and cannot be activated accidentally by an invalid or missing field.

### 12.12 Phase P10 — Add functional aliases and settings

**Purpose:** Add identity/service information needed by CeCoCo after core calls work.  
**Dependencies:** P3; can be implemented before or after P9 if product priorities require it, but do not block the first private-call milestone.

#### Implement classes

```text
FunctionalAliasManager
FunctionalAliasTransaction
FunctionalAliasSnapshot
SettingsManager
SettingsTransaction
SettingsSnapshot
FunctionalAliasCodec
SettingsCodec
```

#### Actions

1. Implement the generic subscription lifecycle if not completed in P6.
2. Subscribe to functional-alias information for MCPTT.
3. Parse one valid alias notification.
4. Map aliases to immutable state.
5. Implement add/remove alias only if the server and product require it.
6. Subscribe to settings.
7. Parse and expose settings state.
8. Map notifications and operation results to Java endpoint callbacks.
9. Test expiry, duplicate notifications, and endpoint stop.

#### Exit criteria

CeCoCo receives the required alias/settings information and operation results, without affecting call state machines.

### 12.13 Phase P11 — Add MCData after MCPTT voice is stable

**Purpose:** Avoid mixing data-session complexity with the first voice implementation.  
**Dependencies:** P3 and stable runtime/transport; voice phases P4–P8 should be passing unless product requirements explicitly prioritize data.

#### Implement classes

```text
McDataIdentity
Conversation
ConversationRegistry
ConversationProcedure
MessageOperation
MessageStateMachine
MessagePayload
SdsMessageProcedure
FileDeliveryProcedure
MessageNotificationPolicy
```

#### Implement in this order

1. Add MCDATA service identity registration only if it uses a distinct identity/profile.
2. Implement one MCData conversation type: private text/SDS or the server’s first required type.
3. Implement conversation creation and termination.
4. Implement outgoing text message.
5. Implement incoming text message.
6. Implement message ID and expiry.
7. Implement required delivery/read notification.
8. Add binary/status/location only when required.
9. Add file delivery/MSRP/HTTP only as a separate phase with its own media/security tests.
10. Map message states to Java `JniMcxMessage*Info` and `JniMcxMessageGroupInfo`.

#### Server tests

- create private conversation;
- send text;
- receive text;
- delivery/read notification;
- rejected message;
- expired message;
- duplicate message;
- conversation termination;
- malformed/oversized payload;
- file delivery only if explicitly enabled.

#### Exit criteria

The approved MCData subset works independently and does not destabilize MCPTT voice calls or registration.

### 12.14 Phase P12 — Add remaining call variants and optional services

**Purpose:** Expand scope only after the first production voice path is proven.

Add one feature at a time:

1. pre-established calls;
2. pre-arranged calls;
3. broadcast calls;
4. listening/ambient calls;
5. first-to-answer calls;
6. ad-hoc calls;
7. MCVideo;
8. off-network/offline behavior;
9. advanced MCData file/location/status procedures;
10. forwarding, transfer, callback, remote group selection, or other special MESSAGE procedures.

For every feature repeat the same mini-cycle:

```text
server capability check
→ standards/profile trace
→ neutral request/event model
→ state machine
→ message/content implementation
→ positive server test
→ negative/timeout/duplicate tests
→ Java compatibility test
→ sanitized trace and review
```

Do not implement these by extending `PrivateCallProcedure` with dozens of flags. Use separate procedure classes sharing common `CallSession` infrastructure.

### 12.15 Phase P13 — Resilience, performance, packaging, and migration

**Dependencies:** All enabled feature phases.

Implement and test:

- registration recovery;
- token refresh;
- SIP transport reconnect;
- server restart;
- network-interface changes;
- multiple endpoint identities;
- simultaneous calls and conversations;
- queue/backpressure limits;
- long-duration calls;
- memory and thread stability;
- sanitizers/fuzzing;
- certificate/key rotation;
- metrics and health checks;
- packaging and rollback;
- feature flag between old and new backend;
- Softil dependency removal.

#### Final migration sequence

1. New core + mock passes JNI contract tests.
2. New core registers an MCPTT identity with the real server.
3. New core completes private signaling-only calls.
4. New core completes private calls with audio.
5. New core completes group calls.
6. New core completes floor/PTT.
7. Additional services are enabled one by one.
8. A controlled pilot runs with rollback.
9. Product, protocol, security, operations, and IP/licensing sign off.
10. Softil includes, link directories, libraries, runtime files, and dead adapter code are removed.

## 13. First implementation sprint: exact task list

If implementation starts now, use this order:

### Sprint task 1 — Create the server profile

Create `docs/server-profile.md` and fill in the actual MCX Server values. Do not commit credentials or private keys. Record references to protected secret locations instead.

**Done when:** transport, TLS, MCPTT identity, authorization mode, registrar/service URI, codec, test destination, registration expiry, and group/affiliation requirement are known.

### Sprint task 2 — Create the core-only build

Create `mcxclient-core` with `McxClient`, `McxRuntime`, `Endpoint`, `EventSink`, IDs, errors, and configuration validation. Do not include JNI, Softil headers, or current vendor libraries.

**Done when:** the core and unit tests build with no JDK and no `third_parties` path.

### Sprint task 3 — Connect the Java gateway to the new mock

Implement the JNI adapter against `MockMcxClient`. Do not connect the real server yet.

**Done when:** endpoint, session, conversation, callbacks, media attach/detach, and shutdown work through the existing Java actors.

### Sprint task 4 — Build the standalone SIP/SDP probe

Implement `SipStack`, `SipTransport`, `SipMessage`, `SipTransaction`, `SipDialog`, SDP parsing, TLS, and sanitized trace output.

**Done when:** the probe can connect to the actual MCX Server and record a safe, replayable transaction.

### Sprint task 5 — Implement MCPTT registration

Implement `McpttIdentity`, `AuthorizationProvider`, `TokenStore`, `RegistrationSession`, and `RegistrationManager`. Start with the simplest server-supported token mode.

**Done when:** the new client registers, refreshes, unregisters, and recovers against the actual server.

### Sprint task 6 — Implement private MCPTT signaling without media

Implement `PrivateCallRequest`, `PrivateCallProcedure`, `CallSession`, SIP dialog handling, MCPTT content, SDP offer/answer, and call event translation.

**Done when:** one outgoing and one incoming private call complete and release against the server with correct Java events.

### Sprint task 7 — Add private-call audio

Implement one codec, RTP/SRTP, jitter, `InProcessMediaPort`, and then `SharedMemoryMediaPort`.

**Done when:** bidirectional audio works through the CeCoCo media bridge and stops cleanly on every release/failure path.

### Sprint task 8 — Implement the minimum group prerequisite

If the server requires affiliation, implement only one-group affiliation and its subscription. Otherwise record the server result and skip to group call setup.

**Done when:** the client has the exact group state required by the server.

### Sprint task 9 — Implement group call setup

Implement one group call type with normal priority and the already tested audio path. Keep PTT disabled until group signaling/media is stable.

**Done when:** group call establishment/release works against the server.

### Sprint task 10 — Implement MBCP/MPC PTT

Implement request/grant/deny/release/revoke and connect floor ownership to transmit media and Java TX/RX events.

**Done when:** the client never transmits without permission and responds correctly to server floor changes.

### Sprint task 11 — Add priority/emergency and identity services

Add priority/emergency under a separate approval, then functional aliases/settings as required by CeCoCo.

**Done when:** each enabled behavior has a dedicated server trace, negative test, security review, and Java compatibility evidence.

### Sprint task 12 — Add MCData and remaining variants

Add text MCData first, then other payloads and call variants one at a time.

**Done when:** voice registration/call/floor regression tests remain green while each additional feature is enabled.

## 14. What should not be implemented first

Avoid these as initial tasks:

- MCVideo;
- all codecs at once;
- all call types;
- emergency alerts before ordinary calls/floor are stable;
- MCData file delivery before basic text messaging;
- full affiliation/alias/settings management before knowing the server prerequisite;
- rewriting the Java actor API;
- porting the old Softil class hierarchy;
- reproducing undocumented vendor timers;
- copying the existing vendor `third_parties` library set into the new build;
- implementing protocol behavior without a server trace or standards/profile justification.

The first completed vertical slice should be:

```text
Configured MCPTT identity
  → authorized token
  → registered at the existing MCX Server
  → outgoing private MCPTT call
  → incoming private MCPTT call
  → negotiated one-codec audio
  → clean release
  → expected Java events
```

Only after this slice passes should the project expand to group calls and floor control.



## 15. Registration implementation in detail: IDMS, tokens, and SIP REGISTER

This section clarifies the most important boundary in the project.

### 15.1 Short answer

**IDMS is normally not the SIP registrar.**

- **IDMS/OIDC/authorization service:** authenticates the user/client and issues an authorization code or access token.
- **SIP registrar/MCX server:** receives SIP `REGISTER`, authenticates/authorizes the SIP contact according to the MCX/IMS profile, and maintains the endpoint’s registration binding.
- **MCX client:** coordinates both procedures but must keep them as separate components.

The relationship is usually:

```text
User/client authorization
        │
        │ HTTPS/OIDC or external authorization service
        ▼
IDMS / authorization server
        │ authorization code or access token
        ▼
MCX client
        │
        │ SIP REGISTER, possibly carrying or referring to MCX authorization
        ▼
SIP registrar / MCX Server
```

The access token may be used in the SIP registration procedure, but that does not make IDMS the SIP registrar.

The exact profile must decide how the token is carried. The current wrapper provides strong evidence of two modes:

```text
setRegisterWithToken(true)
  → RvMc3rdPartyReg
  → token is sent as part of REGISTER

setRegisterWithToken(false)
  → RvMcPublishAuth
  → token/authentication is published separately using PUBLISH
```

The existing JNI comments explicitly describe `true` as “token is sent in REGISTER” and `false` as “token is sent in PUBLISH”. Preserve this as a server-profile decision in the new library; do not hard-code one mode for every deployment.

### 15.2 Recommended first implementation choice

For the first working path, use this architecture:

```text
Java/external CeCoCo authorization service
  → returns authorization code or access token
  → AuthorizationProvider interface
  → native MCX client
  → SIP REGISTER with selected MCX authorization mode
```

This is recommended because the current Java system already contains external MCX authorization actors and the native wrapper already exposes authorization-code/token milestones. It allows the native core to prove MCX registration without immediately reimplementing the complete IDMS/OIDC client.

Use native IDMS HTTP exchange only when one of these is true:

- the product requires the native library to operate without the Java authorization service;
- the existing external service cannot provide the required token lifecycle;
- the MCX Server profile requires a native client-specific token exchange;
- a standalone C/C++ client is a product requirement.

Do not add an HTTP/JSON dependency to `mcxclient-core` merely because the server has an IDMS.

### 15.3 Registration components and responsibilities

Implement these components separately:

| Component | Owns | Does not own |
|---|---|---|
| `AuthorizationProvider` | Requesting an authorization URL/code and obtaining a token through an injected provider | SIP transactions or registrar state |
| `IdmsOAuthClient` | Optional native HTTPS authorization-code/token exchange | SIP registration and MCX call state |
| `TokenStore` | Access token, refresh token if allowed, expiry, service identity, redacted metadata | Token acquisition policy or SIP message construction |
| `ServiceIdentity` | MCPTT identity, service URI, client identity, authorization state | Socket or HTTP implementation |
| `SipRegistrar` | SIP `REGISTER`, refresh, unregister, response handling, registrar route | IDMS login or Java actor lifecycle |
| `McxRegisterPolicy` | Whether authorization is carried in REGISTER or published separately | Low-level SIP parsing |
| `McxAuthPublisher` | MCX PUBLISH authentication procedure when required | General registration state |
| `RegistrationSession` | Per-service registration state, expiry, timers, retries, and recovery | Multiple unrelated endpoints |
| `RegistrationManager` | Coordinates endpoint/service registration and emits snapshots/events | Parsing every MCX body |
| `RegistrationEventTranslator` | Converts neutral events to Java/JNI events | Making protocol decisions |
| `CredentialProvider` | Secure certificate/key/token references | Logging or exposing secrets |

Recommended relationships:

```text
Endpoint
 ├── ServiceIdentity(MCPTT)
 ├── AuthorizationProvider
 ├── TokenStore
 ├── McxRegisterPolicy
 ├── SipRegistrar
 ├── McxAuthPublisher (optional by profile)
 └── RegistrationSession
```

### 15.4 Exact registration sequence

The implementation should support the following stateful sequence. Some deployments may skip the authorization-code steps when a valid token is supplied externally.

#### Path A — External authorization provider, recommended first

```text
1. Configure MCPTT ServiceIdentity
2. Create endpoint and RegistrationSession
3. Request authorization URL if token is absent/expired
4. Send URL to existing Java/external authorization service
5. Receive authorization-code acknowledgement
6. Receive authorization code
7. AuthorizationProvider exchanges code externally
8. Receive access token and expiry
9. Store token securely in TokenStore
10. Configure McxRegisterPolicy
11. Create SIP transport and SipRegistrar
12. Send SIP REGISTER with token mode required by profile
13. Handle 401/407 challenge if applicable
14. Receive successful REGISTER response
15. Emit REGISTERED snapshot/event
16. Schedule refresh before expiry
17. Refresh or unregister on shutdown
```

#### Path B — Native IDMS/OIDC exchange

```text
1. Configure MCPTT ServiceIdentity and IDMS endpoints
2. Create authorization URL/request
3. Obtain authorization code through the approved user/device flow
4. IdmsOAuthClient sends HTTPS token request
5. Validate HTTPS certificate and hostname
6. Parse token response
7. Store access/refresh token and expiry
8. Continue with the same SIP registration path as Path A
```

#### Path C — Preconfigured token for a probe/test profile

```text
1. Load token through protected CredentialProvider
2. Validate token metadata if available
3. Do not attempt to obtain an authorization code
4. Continue with the same SIP registration path
```

Path C is useful for the first registration probe but must be an explicit mode, not a silent production fallback.

### 15.5 Detailed implementation tasks for registration

#### REG-01 — Define the server registration profile

Create a profile containing:

```text
mcptt_service_uri
sip_registrar_uri
sip_proxy_uri
local_sip_uri
transport: udp | tcp | tls
server_certificate_policy
client_certificate_reference, if required
idms_authorization_endpoint, if used
idms_token_endpoint, if used
authorization_mode: external | native | preconfigured
register_authorization_mode: token_in_register | token_in_publish
registration_expiry
refresh_margin
keepalive_interval
request_timeout
retry_policy
```

Tasks:

1. Obtain actual values from the existing MCX Server deployment.
2. Identify which values are server addresses versus IDMS addresses.
3. Record whether the registrar and IDMS use the same hostname; do not infer that they are the same service.
4. Record whether the token is opaque to the client or whether the profile requires claims to be inspected.
5. Record the expected successful response and expiry.
6. Store no token or private key in the profile file.

**Done when:** A reviewer can identify the IDMS endpoint and SIP registrar endpoint separately and explain the token path between them.

#### REG-02 — Implement `AuthorizationProvider`

Define a dependency-free port in the core, conceptually:

```text
requestAuthorizationUrl(service_identity) -> operation_id
submitAuthorizationCode(service_identity, code) -> operation_id
setAccessToken(service_identity, token, expiry) -> operation_id
clearToken(service_identity)
```

Tasks:

1. Return operation IDs immediately.
2. Emit authorization URL/code-requested events asynchronously.
3. Accept authorization code or token from Java/external code.
4. Validate non-empty values and expiry.
5. Never log the code or token.
6. Emit token available/rejected/expired events.
7. Support one independent state per service identity.
8. Make token replacement atomic so registration never uses a partially updated token.

**Done when:** The core can receive a token from a fake provider and transition to `TOKEN_AVAILABLE` without knowing HTTP or Java.

#### REG-03 — Implement optional native `IdmsOAuthClient`

Only implement this task if the server/product requires native token exchange.

Recommended third parties:

| Need | Recommendation | Scope |
|---|---|---|
| HTTPS/OIDC HTTP | `libcurl` | Optional adapter, not core; use its TLS verification and timeout APIs |
| TLS/crypto | OpenSSL through libcurl and/or the approved system TLS stack | Certificate validation, protected key handling, cryptographic policy |
| JSON token response | A pinned JSON parser such as `nlohmann/json`, `yyjson`, or `RapidJSON` after license/performance review | Parse token response only; do not use it as a JWT trust decision by default |
| URL/form encoding | libcurl utilities or a small bounded project utility | Authorization-code token request |

Tasks:

1. Implement authorization-code exchange with HTTPS POST.
2. Configure CA verification and hostname verification.
3. Set connection, total-request, and response-size limits.
4. Send only the required OAuth fields.
5. Parse `access_token`, `refresh_token` if allowed, `expires_in`, `token_type`, and server error fields.
6. Treat the access token as opaque unless the approved profile explicitly requires local claim processing.
7. Do not validate a JWT signature locally as a replacement for server authorization unless requirements explicitly require it.
8. Redact Authorization headers and response bodies from logs/traces.
9. Test HTTP redirects, TLS failure, malformed JSON, missing fields, expired code, invalid client, and timeout.
10. Make refresh policy explicit and bounded.

**Done when:** A native test client can exchange a controlled authorization code for a token without leaking credentials and hands the token to the same `TokenStore` used by the external-provider path.

#### REG-04 — Implement `TokenStore`

Fields to manage:

```text
service_identity
access_token: protected value
refresh_token: protected value, if allowed
issued_at
expires_at
token_type
source: external | native | preconfigured
```

Tasks:

1. Keep token values out of ordinary string descriptions and `EndpointSnapshot`.
2. Store expiry using a monotonic/absolute-time policy that handles clock skew.
3. Treat expired tokens as unavailable before starting REGISTER.
4. Support atomic replacement and invalidation.
5. Do not persist refresh tokens until storage/security approval exists.
6. Wipe or release sensitive buffers according to the selected C++ security policy.
7. Expose only redacted metadata to diagnostics.

**Done when:** Tests prove that token contents do not appear in logs, snapshots, exceptions, traces, or normal core dumps generated by the test harness.

#### REG-05 — Implement SIP transport and `SipRegistrar`

For the first registration implementation, the recommended candidate is **PJSIP**, because:

- the workspace already contains a PJSIP-based MCX signalling project;
- it provides SIP parsing, transports, transaction/dialog support, authentication helpers, and registration-client functionality;
- it can be wrapped behind a project-owned `SipRegistrar` interface;
- it can later support INVITE, SUBSCRIBE, PUBLISH, and dialog procedures needed by MCX.

PJSIP is a candidate, not an automatic final decision. Confirm its exact version, build options, event-loop integration, license model, TLS backend, and ABI policy before adoption. For a proprietary product, perform the required PJSIP licensing review; do not assume the default license is sufficient.

For registration-only P2/P3, use the smallest useful PJSIP components:

```text
pjlib
pjlib-util
pjsip core
pjsip transaction/endpoint/transport modules
pjsip-ua registration/auth support
PJSIP TLS transport if required
```

Do not add `pjmedia`, codecs, ICE, or video until P5 requires them.

`SipRegistrar` responsibilities:

1. Build REGISTER from the neutral identity/profile.
2. Set Request-URI, To, From, Call-ID, CSeq, Contact, Via, Expires, Route, and required MCX headers/body.
3. Apply token-in-REGISTER policy when selected.
4. Ask `SipAuthenticator` to handle profile-required challenges.
5. Correlate transaction responses to `RegistrationSession`.
6. Schedule refresh before expiry.
7. Send unregister with `Expires: 0` or the profile-required equivalent.
8. Rebuild registration after transport/server failure.
9. Expose server status/reason without exposing credentials.
10. Ensure only one registration transaction is active per endpoint/service identity.

**Done when:** A standalone registrar test can send a valid registration transaction to the existing server and produce a sanitized request/response/event timeline.

#### REG-06 — Implement the MCX token placement policy

Create a profile-owned policy rather than a boolean spread through the code:

```text
enum McxRegisterAuthorizationMode {
    TokenInRegister,
    TokenInPublish
};
```

Tasks:

1. Map the current Java `setRegisterWithToken(true/false)` compatibility value to this neutral mode in the JNI adapter.
2. For `TokenInRegister`, attach the token using the exact MCX/IMS profile format required by the server.
3. For `TokenInPublish`, send the required MCX PUBLISH procedure before or alongside registration according to the server trace.
4. Keep the access token out of generic SIP logging.
5. Do not assume PUBLISH is ordinary presence publication; model it as `McxAuthPublisher` with typed configuration.
6. Record the mode in `server-profile.md`.
7. Test wrong-mode behavior and ensure failure is explicit.

**Done when:** Both policy modes are represented in the design, but only the mode proven by the actual MCX Server is enabled for the first product profile.

#### REG-07 — Implement registration state and recovery

Implement:

```text
NOT_CONFIGURED
AUTHORIZATION_REQUIRED
TOKEN_REQUESTED
TOKEN_AVAILABLE
REGISTERING
REGISTERED
REFRESHING
UNREGISTERING
UNREGISTERED
FAILED
RECONNECTING
```

Tasks:

1. Reject registration without an authorized MCPTT token unless the server profile explicitly allows another mode.
2. Prevent duplicate REGISTER requests.
3. Refresh before expiry with a configurable margin.
4. Handle 401/403 differently from transport timeout and server 5xx.
5. Invalidate a rejected token when the server indicates token failure.
6. Retry transport failures with bounded backoff.
7. Stop retrying after explicit endpoint shutdown.
8. Re-register after server restart or network-interface change.
9. Emit exactly one terminal result per registration operation.
10. Preserve Java-visible authorization and registration events.

**Done when:** Failure-injection tests cover every state and no registration attempt can survive endpoint shutdown.

### 15.6 Which third parties are needed for REGISTER?

For SIP registration itself:

| Dependency | Needed for REGISTER? | Recommendation |
|---|---:|---|
| SIP stack | Yes | Start evaluation with PJSIP; wrap it behind `SipRegistrar` |
| TLS library | Only for SIP/TLS | OpenSSL through the selected SIP stack/system integration |
| DNS resolver | If registrar/proxy uses hostname | Use the SIP stack/system resolver first; add c-ares only if required |
| JSON parser | No | Not needed for SIP REGISTER itself |
| libcurl | No | Not needed for SIP REGISTER itself |
| XML parser | Only if the selected REGISTER body/profile carries XML | Add libxml2/Expat/pugixml only after trace/profile confirmation |
| SRTP/libsrtp2 | No | Needed later for secure media, not registration |
| Codec library | No | Needed later for RTP audio |
| JNI/JDK | No for core | Needed only in `mcxclient-jni` |
| GoogleTest | Test-only | Recommended for registration/state/adapter tests |
| SIPp/Wireshark/tcpdump | Diagnostic/test tools | Optional external tools, not runtime dependencies |

For IDMS token exchange:

| Dependency | Needed? | Where it belongs |
|---|---:|---|
| libcurl | Only if native code exchanges the authorization code for a token | Optional `IdmsOAuthClient` adapter |
| OpenSSL | Yes for HTTPS security, directly or through libcurl | TLS/credential adapter |
| JSON parser | Usually yes for token endpoint JSON | Optional `IdmsOAuthClient`; pinned version |
| JWT library | Usually no | Treat access token as opaque unless profile requires local verification/claims |
| Browser/device-flow library | Usually no | Keep user interaction in Java/external authorization service initially |

### 15.7 Recommended dependency profiles

#### Profile A — First implementation, external IDMS owner

Use this first when the existing Java authorization service already obtains tokens:

```text
mcxclient-core
  + PJSIP or selected SIP stack
  + OpenSSL if SIP/TLS is required
  + selected XML parser only if REGISTER content needs it
  + GoogleTest for tests
  + optional SIPp/Wireshark for diagnostics
```

No `libcurl` or JSON parser is required in the core or registration path.

#### Profile B — Standalone native client with IDMS exchange

Use this only when native ownership is required:

```text
mcxclient-core
  + PJSIP
  + OpenSSL
  + libcurl
  + pinned JSON parser
  + GoogleTest
```

Keep `libcurl` and the JSON parser behind `IdmsOAuthClient`. The `SipRegistrar` should receive a token through `TokenStore` and must not call libcurl directly.

#### Profile C — Later voice/media build

Add only after P4 private signaling works:

```text
PJSIP media or project-owned RTP adapter
+ libsrtp2 if required
+ one approved codec
+ jitter implementation
+ shared-memory media adapter
```

Do not add all current vendor codecs or media libraries at once.

### 15.8 Registration server test matrix

Run these tests against the existing MCX Server and store sanitized evidence:

| ID | Scenario | Expected result |
|---|---|---|
| REG-T01 | Valid external token, token in REGISTER | Registered if server profile supports this mode |
| REG-T02 | Valid external token, token in PUBLISH | Registered if server profile supports this mode |
| REG-T03 | Wrong token placement mode | Explicit server rejection and neutral failure |
| REG-T04 | Expired access token | No REGISTER or explicit authentication failure, according to profile |
| REG-T05 | Invalid authorization code | IDMS failure; no SIP registration attempt |
| REG-T06 | IDMS HTTPS certificate failure | Token exchange fails securely |
| REG-T07 | SIP TLS certificate failure | Registration fails securely |
| REG-T08 | SIP 401/407 challenge | Authenticated retry or explicit failure |
| REG-T09 | SIP 403/404 | No uncontrolled retry; correct reason |
| REG-T10 | Registration success | Registered state, expiry, contact, and event timeline |
| REG-T11 | Registration refresh | One refresh before expiry and stable state |
| REG-T12 | Server restart | Bounded recovery and re-registration |
| REG-T13 | Network disconnect | Reconnect/backoff and no leaked transaction |
| REG-T14 | Explicit unregister | Final unregistered state and no further retry |
| REG-T15 | Endpoint stop during REGISTER | Transaction canceled and no callback after destruction |
| REG-T16 | Two simultaneous register commands | One accepted operation or deterministic rejection |

### 15.9 Registration acceptance checklist

Do not mark registration complete merely because a `200 OK` was received. Confirm:

- the token was acquired through the approved IDMS/external path;
- the IDMS endpoint and SIP registrar endpoint are separately documented;
- the selected token placement mode is proven by server evidence;
- SIP/TLS certificate validation is active;
- the `REGISTER` response is correlated to the correct endpoint/service identity;
- expiry and refresh are implemented;
- unregister is implemented;
- 401/403, timeout, 5xx, TLS failure, and invalid-token behavior is tested;
- token values are absent from logs, traces, snapshots, and exceptions;
- Java receives authorization and registration events in the expected order;
- the native core does not require Java when using a test `AuthorizationProvider`;
- the core does not depend on libcurl/JSON unless native IDMS exchange was explicitly selected.

## 16. Third-party usage by implementation phase

This matrix prevents adding the entire current vendor dependency set to the new project.

| Phase | Runtime dependencies | Optional/test dependencies | Explicitly not needed yet |
|---|---|---|---|
| P0 server profile | None | Wireshark/tcpdump, SIPp, existing black-box client | Any vendor SDK |
| P1 core/mock | C++ standard library | GoogleTest, sanitizers | SIP, TLS, HTTP, codecs |
| P2 SIP/SDP/TLS probe | PJSIP candidate, OpenSSL for TLS | SIPp, Wireshark, libpcap | RTP codecs, SRTP, libcurl |
| P3 MCPTT registration | PJSIP, OpenSSL if SIP/TLS | libcurl + pinned JSON parser only for native IDMS; GoogleTest | pjmedia, codecs, libsrtp2 |
| P4 private signaling | PJSIP SIP/UA, SDP support, XML parser only if required by MCX body | Trace/replay tools | Full media engine, all codecs, floor engine |
| P5 private audio | RTP/media adapter, libsrtp2 if required, one approved codec | Media generators, packet-loss test tools | Video, unrelated codecs, BFCP/MSRP unless profile requires them |
| P6 affiliation | PJSIP SUBSCRIBE/PUBLISH, XML/content parser | XML fuzzing, server replay | MCData file transfer |
| P7 group call | Existing SIP/SDP/media dependencies | Group scenario simulator | Full floor race implementation |
| P8 floor/PTT | RTP/RTCP support, MBCP/MPC project code, libsrtp2 as required | Packet replay and race simulator | MCVideo transmission control unless required |
| P9 priority/emergency | Existing SIP/MCX/XML/TLS dependencies | Security/fuzz/audit tooling | New HTTP dependency unless emergency profile requires it |
| P10 aliases/settings | Existing SUBSCRIBE/PUBLISH/XML dependencies | Content vector tests | MCData stack |
| P11 MCData | SIP MESSAGE and/or MSRP/HTTP dependencies selected by profile | File-transfer test server, JSON/XML tools as required | MCVideo |
| P12 variants | Only dependencies required by each selected variant | Dedicated interoperability tools | Unapproved optional stacks |
| P13 hardening | Pinned production runtime dependencies | ASan/UBSan/TSan, fuzzers, SBOM/CVE scanners | Vendor Softil libraries |

### 16.1 Dependency selection rule

For each dependency, record:

```text
name
exact version
source/hash
license
CMake target
runtime or test-only
feature phase
security owner
upgrade policy
removal/adapter boundary
```

The preferred first stack is:

```text
PJSIP       → SIP transactions, dialogs, registration, SDP foundation
OpenSSL     → SIP/TLS and certificate/key policy
libcurl     → optional native IDMS HTTPS only
JSON parser → optional native token-response parsing only
GoogleTest  → unit/contract/state tests
SIPp/tools  → optional external SIP test/replay tools
```

This recommendation remains subject to the project’s license review, exact server profile, supported compiler, and interoperability testing. PJSIP is not an MCX implementation; it only supplies lower-level SIP/SDP/transport capabilities. MCPTT procedures, token placement, MCX content, group calls, floor control, and CeCoCo events remain project code.
