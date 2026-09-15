# Roadmap: Softil-independent MCX protocol library for the CeCoCo gateway

## 0. Purpose and non-negotiable boundaries

This document is the implementation roadmap for building an independent native C++ **MCX protocol library** to run inside the CeCoCo MCX gateway. The library will replace the Softil/BEEHD-dependent native implementation that currently connects the gateway to the MCX Server.

CeCoCo already has an established SIP-facing integration. The backend and Asterisk communicate with the MCX gateway through the existing SIP path, including the current `X-ccm-*` headers, Java actors, and gateway connector behavior. That path is **not** the initial replacement target. The new library begins behind the gateway's Java/JNI boundary and implements the MCX-server-facing protocol behavior.

The intended boundary is:

```text
CeCoCo backend / Asterisk
        │
        │ Existing CeCoCo SIP integration
        │ SIP INVITE + X-ccm-* headers
        ▼
CeCoCo MCX gateway
(Java actors + existing JNI compatibility boundary)
        │
        │ New independent native MCX protocol library
        │ MCX procedures over the server-facing SIP/IMS stack
        │ MCX content + SDP + RTP/SRTP/media
        ▼
MCX Server
```

In this roadmap, **MCX client** describes the protocol role performed by the library toward the MCX Server; **MCX protocol library** describes the implementation artifact; and **MCX gateway** describes the CeCoCo host component that exposes the existing SIP-facing integration and embeds the library. This distinction is mandatory throughout the design.

The goal is **not** to copy Softil, reproduce its private classes, replace CeCoCo's SIP connector, or build another generic SIP wrapper. The goal is to implement the standards- and profile-based MCX behavior required by the current CeCoCo product behind a stable, vendor-neutral library API and the existing JNI adapter.

Use three sources of truth:

1. **The current CeCoCo contract**: Java/JNI methods, callbacks, object lifetime, event ordering, media-buffer contract, existing SIP-facing integration, and configuration flow.
2. **Published standards and authorized project specifications**: MCX/MCPTT procedures, SIP/IMS, SDP/RTP, FRMCS requirements, security, and the exact MCX Server profile.
3. **Interoperability evidence**: sanitized traces from the gateway's server-facing MCX traffic and controlled tests against the MCX Server. CeCoCo's existing SIP traffic is used to preserve the gateway boundary, not to define MCX protocol semantics.

Do not use Softil headers, binaries, generated code, proprietary implementation details, or copied source as the design specification. The existing Softil wrapper is useful for discovering responsibilities and observable behavior; it is not a legal or technical substitute for the applicable standards.

> Important: the repository configuration files inspected during this analysis contain credential/token-like values. Do not copy them into the new project, documentation, packet captures, tests, or commits. Rotate exposed credentials according to the project security process.

## 1. What you are building

The replacement is a reusable native MCX protocol library embedded in the existing CeCoCo MCX gateway. It is not a standalone replacement for the gateway's SIP-facing side. Its primary responsibility is to translate neutral gateway/JNI commands into MCX protocol procedures toward the MCX Server and translate MCX responses/events back into the existing gateway/JNI contract.

The intended implementation shape is:

```text
CeCoCo backend / Asterisk
        │
        │ Existing SIP-facing path; unchanged initially
        ▼
McxEndpointActor / McxSessionActor
        │
        │ Existing Java API and gateway behavior
        ▼
Thin JNI compatibility adapter
        │
        │ Neutral commands/events/media ports
        ▼
Independent MCX protocol library
 ┌─────────────────────────────────────────────┐
 │ Client runtime and lifecycle                │
 │ Identity, authorization, registration      │
 │ SIP transactions, dialogs, authentication  │
 │ SDP offer/answer                            │
 │ MCX service procedures and content          │
 │ Call/session state machines                 │
 │ Floor-control state machine                 │
 │ RTP/SRTP, codecs, jitter and media ports   │
 │ Subscriptions, affiliation and aliases     │
 │ Messaging/conversations                     │
 │ Emergency and priority procedures           │
 │ Timers, recovery, logging and metrics      │
 └─────────────────────────────────────────────┘
        │
        ▼
MCX Server
```

A SIP library can provide message parsing, transactions, dialogs, authentication, and perhaps SDP/RTP support. It will not automatically provide MCX procedures, MCPTT service semantics, floor control, affiliation, functional aliases, emergency behavior, or CeCoCo-compatible callbacks. Treat it as a lower-level dependency, not as the replacement itself.

## 2. Current system: verified source inventory

### 2.1 Repository locations

- Native gateway: `C:\Git\F_CeCoCo\MCX\F700950-gateway-mcx-lib`
- Java MCX multiproject: `C:\Git\F_CeCoCo\MCX\cecoco-all-mcx`
- Java connector: `cecoco-gw-mcxconnector-lib`
- Deployable gateway: `cecoco-mcx-sip-gateway`
- Existing native mock: `F700950-gateway-mcx-lib\src\jni_mock`
- Softil-dependent wrappers: `F700950-gateway-mcx-lib\src\softil`
- JNI base and Java-facing native contract: `F700950-gateway-mcx-lib\src\jni_base`
- Gateway registration documentation: `cecoco-mcx-sip-gateway\doc\readme.md` and `doc\register-sequence-diagram.html`

### 2.2 Current native build boundary

The current CMake project builds three conceptual variants:

| Variant | Current source path | Current purpose | Replacement direction |
|---|---|---|---|
| Real gateway | `src/jni_impl` + `src/softil` | JNI adapter plus Softil wrappers | Keep JNI adapter; replace the implementation behind it |
| Mock gateway | `src/jni_mock` | Test without Softil | Make this a formal contract/mock backend, not only a test shortcut |
| Application | `src/softil` and test application sources | Native/manual experiments | Replace with a standalone probe and protocol test tools |

The current real target links a large vendor stack covering SIP, SDP, RTP/RTCP, MSRP, BFCP, ICE/STUN/TURN, media, codecs, TLS/crypto, XML and MCX/BeeHD components. The independent project must select and own its replacement dependencies explicitly. Do not carry the vendor link list forward automatically.

### 2.3 Verified Java/JNI lifecycle

The Java side exposes these broad layers:

- `JniMcxGateway`: configure, start/stop, logging, shared audio setup, and incoming ad-hoc range configuration.
- `JniMcxEndPoint`: configure an identity and service URLs, start/stop, register/unregister, obtain an authentication URL, submit authorization code, make calls/conversations, subscribe, affiliate, manage aliases/settings, and send emergency alerts.
- `JniMcxSession`: get call ID, answer/reject/hang up, push/release PTT, attach player/recorder devices, and change call priority.
- `JniMcxConversation`: create/terminate a conversation and exchange MCData/messages through the conversation actor contract.
- `JniClassFactory`: attach/detach Java objects to native counterparts and maintain native object handles.

The JNI layer is asynchronous. Java calls generally start an operation; native callbacks later become Java methods or actor messages. The replacement must preserve that property even if the internals are completely different.

### 2.4 Verified endpoint responsibilities

The current endpoint model maintains one endpoint identity with service-specific information for MCPTT, MCDATA, and MCVIDEO. The current Java enum uses numeric service types 1, 2, and 3. Treat those numbers as a compatibility detail at the JNI boundary, not as the domain model.

The endpoint currently participates in:

1. **Initialization** from an identity configuration section and per-service URLs.
2. **Authorization state** per service type:
   - authentication-code request;
   - request acknowledged;
   - token request;
   - token response.
3. **Registration/unregistration** callbacks.
4. **Subscription** to affiliation, functional-alias, and settings information.
5. **Affiliation/de-affiliation** operations with operation IDs and group information.
6. **Functional-alias add/remove** operations with operation IDs and alias information.
7. **Incoming affiliation and functional-alias information** represented as XML/application content at the current boundary.
8. **Emergency alert receive/send** with normal or ad-hoc information, location XML, criteria, aliases, priorities, and received-indicator flags.
9. **Call and conversation creation/destruction** notifications.
10. **Incoming callback request** notifications.

The independent library should model these as explicit domain services and transactions, rather than reproducing `IdentityStore`, `McxIdentityInfo`, and operation classes as a one-to-one copy.

### 2.5 Verified session responsibilities

The current session carries demanded call information including, depending on call type:

- local and remote party URIs;
- local and remote functional aliases;
- incoming/outgoing direction;
- call type, including private, group, broadcast, listening, pre-established, pre-arranged, chat, and ad-hoc variants;
- simplex/duplex media;
- hook/commencement mode;
- floor-control priority;
- implicit PTT request;
- call priority and SIP/resource priority;
- emergency-alert indicators;
- ad-hoc criteria or member list;
- calling user ID.

The Java session observes these state families:

```text
Call/session: INVITING, OFFERING, PROCEEDING, CONNECTED,
              DROPPING, DISCONNECTED, REJECTING,
              AUTHENTICATING, REDIRECTED

Receive/transmitting: RX_OFF, RX_ON, TX_OFF, TX_ON

Priority: normal, imminent peril, emergency
```

The actual enum values and event ordering must be treated as compatibility data and frozen in contract tests before migration. In particular, the current adapter translates priority numbering between the Java model and the vendor model. The new library must use a neutral priority enum and perform any conversion only in the compatibility adapter.

### 2.6 Verified media responsibilities

The current gateway configures a shared-memory/circular-buffer audio path through:

```text
setAudio(key, bufferSize, bufferCount, jitterSize, clean)
```

A per-call session can attach a player and recorder device. The current `CircularBuffer` abstraction has supplier/consumer roles, media frames, a control buffer, a shared write offset, jitter configuration, pause/resume behavior, and media activation events.

The new library should not depend on shared memory internally. Define a neutral `MediaPort` interface first, then implement:

- an in-process frame-queue port for unit/integration tests;
- a shared-memory compatibility port for the current CeCoCo/Asterisk bridge;
- optional RTP-direct or file/test ports for diagnostics.

This keeps the MCX protocol stack independent from the current audio transport.

### 2.7 Verified configuration clues

The existing configuration demonstrates that the deployed profile can involve:

- local signaling and RTP addresses/ports;
- UDP, TCP, and TLS SIP transport choices;
- TLS certificate/verification settings;
- registration expiration and retry timers;
- SIP session timers and keepalives;
- RTP port ranges and packet-buffer sizing;
- AMR-NB, AMR-WB, G.711, G.722, EVS and other codec switches;
- SRTP/SRTCP settings;
- MCPTT, MCDATA and MCVIDEO service identities and client IDs;
- MCX service, affiliation, functional-alias and settings URLs;
- MCX/MBMS/MCPTT timer and floor-related settings;
- IDMS/OAuth authorization and token endpoint information.

These are observations about the existing deployment, not a complete requirements list. Convert them to a versioned neutral configuration schema and validate each field against the target MCX Server and approved specifications.

## 2.8 Reading guide: how to learn the behavior to implement

The phrase “an MCX library in the role currently served by Softil” must be interpreted as **a library that interoperates with the target MCX Server and preserves the current CeCoCo gateway contract**. It must not mean reproducing Softil’s API, class hierarchy, internal algorithms, or proprietary wire behavior. Read the documents below in layers and record the exact edition/release used by the project.

### 2.8.1 Read the project profile first

Read these local, controlled documents before selecting protocol features:

1. `FRMCS FIS FIS-7970 v2.1.0` — functional interface behavior and FRMCS-specific procedures.
2. `FRMCS SRS AT-7800 v2.1.0` — system scope, roles, security, identity, service, and deployment requirements.
3. `FRMCS FRS FU-7120 v2.1.0` — functional requirements and operational use cases, including communication, floor/PTT, emergency, and railway applications.
4. `FRMCS FFFIS-7950 v2.1.0` — form-fit and functional interface constraints relevant to the client boundary.
5. `TOBA FRS TOBA-7510 v2.1.0` — trackside/on-board gateway context where the client is used through FRMCS equipment.
6. The currently approved AT-7800/FIS/FRS revision for the target product release. The indexed AT-7800 v2.2.0 must be treated as a possible newer baseline, not mixed silently with the v2.1.0 documents.

For each feature, distinguish **mandatory**, **optional**, **informative**, and version-qualified requirements such as `M-V3` or `O-Vx`. An FRMCS requirement may constrain the product profile without defining every SIP message; the referenced 3GPP Stage 2 and Stage 3 specifications remain necessary.

### 2.8.2 Read the standards in implementation order

| Reading stage | Standards/documents to read | Questions to answer before coding |
|---|---|---|
| 1. MCX common architecture | 3GPP TS 22.280 and TS 23.280 | Which identity types, service identities, group identities, functional aliases, authorization relationships, and common procedures exist? |
| 2. MCPTT service model | 3GPP TS 22.179 and TS 23.379 | Which call types, group/private/broadcast procedures, priority, emergency, affiliation, floor, late entry, and ad-hoc behavior are in scope? |
| 3. MCPTT protocol | 3GPP TS 24.379 | Which SIP methods, headers, content types, XML bodies, transactions, dialogs, floor messages, and timers implement each selected Stage 2 procedure? |
| 4. MCData | 3GPP TS 22.282, TS 23.282, and TS 24.282 | Which conversation, messaging, file/location/status, and data-session procedures are required, and which are outside the MVP? |
| 5. MCVideo, if required | 3GPP TS 22.281, TS 23.281, and TS 24.281 | Which video session, media, floor, and codec procedures are actually required by the target profile? Do not include MCVideo merely because the identity model supports it. |
| 6. MCX security | 3GPP TS 33.180, with the IMS security references such as TS 33.203 | How are the MC user authenticated, service-authorized, tokens handled, signaling protected, and media/data confidentiality and integrity provided? |
| 7. IMS/SIP control | 3GPP TS 24.229 plus the applicable IETF SIP/SDP RFCs | How are REGISTER, authentication, dialogs, provisional responses, session timers, UPDATE/re-INVITE, content negotiation, and error handling realized in the selected network profile? |
| 8. Media and transport | 3GPP media/codec specifications selected by the server profile; IETF RFC 3550, RFC 3711, RFC 4566/RFC 8866, RFC 5764, and applicable RTP/RTCP guidance | Which codecs, payload types, packetization, SRTP profile, RTCP behavior, direction, jitter, and network transports are required? |
| 9. FRMCS railway profile | The approved FIS/SRS/FRS/FFFIS documents and applicable ETSI FRMCS documents, including the referenced TS 103 765-3 and TS 103 765-4 procedures | How are the generic MCX procedures constrained by on-board/trackside roles, railway applications, addressed/initiating areas, REC, domain, and deployment behavior? |

The table is a reading baseline, not permission to implement every feature in every referenced specification. The approved product profile and the target MCX Server capability matrix decide the supported subset.

### 2.8.3 Read the general protocol specifications selectively

Do not read SIP or RTP as a substitute for MCX knowledge. Read only the parts needed by the selected procedures, while maintaining a protocol dependency list. The initial list normally includes:

- SIP transactions and dialogs: RFC 3261; reliable provisional responses (RFC 3262); UPDATE (RFC 3311); caller identity/privacy (RFC 3325 and applicable profile rules); Reason (RFC 3326); SIP session timers (RFC 4028); and the project-required authentication, transport, and error extensions.
- SDP offer/answer: RFC 3264 and the current SDP syntax specification, RFC 8866. Validate direction, media sections, payload formats, connection information, and renegotiation rules rather than comparing only text formatting.
- RTP/RTCP and security: RFC 3550, RFC 3551 where applicable, RFC 3711 for SRTP, and RFC 5764 when DTLS-SRTP is used. The MCX/FRMCS security profile takes precedence over a generic media-library default.
- Data and messaging transports: the transport explicitly selected by the MCData procedure and server profile, such as MSRP-related specifications where required. Do not assume that a generic SIP MESSAGE implementation covers MCData.
- TLS and credential handling: the TLS version/profile, certificate validation, hostname policy, protected key storage, and token lifecycle required by the approved security profile. Never turn off certificate validation to make an interoperability test pass.

For every RFC or 3GPP reference, record whether it is **normative for the selected feature**, a referenced dependency, or merely background reading. Record the applicable 3GPP release and any FRMCS profile deviations.

### 2.8.4 Use the repository material as behavior evidence, not as the specification

After the normative reading, inspect the existing implementation and traces in this order:

1. `F700950-gateway-mcx-lib\doc\CLASS_INDEX.md` — map responsibilities such as identity, registration, calls, subscriptions, affiliation, aliases, messages, media, and callbacks to neutral modules.
2. `cecoco-communications\docs\sequence-diagrams.md` — extract observable SIP/MESSAGE/INVITE/PUBLISH/SUBSCRIBE flows, content fields, and CeCoCo event timing.
3. The Java JNI base and implementation classes — freeze method signatures, callback order, object lifetime, error mapping, and compatibility values.
4. Sanitized SIP/SDP/MCX/RTP captures — confirm what the target server actually sends and accepts; never treat one trace as proof of a general rule.
5. The Softil Programmer Guide and Softil wrapper sources — use only to understand integration boundaries, configuration ownership, and observable callbacks. Do not use proprietary names or implementation details as requirements, and do not copy its wire output without a standards/profile justification.

For each behavior discovered in the code or a trace, label it as one of:

- **Standard requirement** — directly supported by an approved normative clause.
- **FRMCS profile requirement** — required by FIS/SRS/FRS/FFFIS or an approved railway profile.
- **MCX Server interoperability behavior** — required by the tested server but not yet explained by a normative clause; investigate and document it.
- **CeCoCo compatibility behavior** — required by Java actors, JNI, media, or deployment contracts.
- **Vendor observation** — seen in Softil but not yet justified; do not implement it as fact.

The last category must remain in an investigation backlog until it has a standards, profile, or controlled interoperability justification.

## 2.9 Standards follow-up process

Create `docs/standards-baseline.md` and keep it under change control. It is the living contract between requirements, protocol implementation, and interoperability testing.

### 2.9.1 Required traceability record

For every supported feature, maintain one row with these fields:

```text
Feature / use case
FRMCS requirement and exact revision
3GPP Stage 1 reference, if applicable
3GPP Stage 2 procedure and clause
3GPP Stage 3 procedure and clause
IMS/SIP/SDP/RTP/security dependency
Target MCX Server profile/version
CeCoCo/JNI compatibility requirement
Implementation module and state machine
Positive test vector and expected events
Negative/interoperability test
Open interpretation or deviation
Owner, review date, and status
```

A feature is not “implemented” when a class exists. It is implemented only when the selected procedure, messages/content, state transitions, timer/error behavior, security implications, and tests are all traced.

### 2.9.2 Review gates

Use these gates for every new procedure or standards update:

1. **Scope gate** — confirm that the feature is required by the approved product profile and identify the exact call/service variant.
2. **Version gate** — record the 3GPP release, FRMCS document version, server version, and applicable profile. Do not combine clauses from different releases without an explicit compatibility decision.
3. **Procedure gate** — draw the Stage 2 sequence, then annotate every Stage 3 SIP method, header, body, transaction, timer, and response.
4. **Security gate** — identify identity, authorization, credential, token, TLS, signaling, media, privacy, and logging requirements.
5. **API gate** — map the procedure to neutral commands/events and to the existing JNI contract without leaking vendor types.
6. **Test gate** — create parser/content vectors, state-machine tests, duplicate/late/failure tests, and a controlled server interoperability scenario.
7. **Interoperability gate** — capture a sanitized trace, compare behavior against the profile, document deviations, and obtain review before enabling the feature by default.

### 2.9.3 Standards watch list

Review this list at each product release, dependency upgrade, and MCX Server upgrade:

- New 3GPP release or corrigendum affecting TS 23.280, TS 23.379/24.379, TS 23.282/24.282, TS 23.281/24.281, or TS 33.180.
- New FRMCS revision of AT-7800, FIS-7970, FU-7120, FFFIS-7950, TOBA-7510, or a referenced ETSI FRMCS document.
- MCX Server changes to supported procedures, content schemas, identities, codecs, security algorithms, timers, or SIP/SDP behavior.
- IETF updates affecting SIP, SDP, RTP, SRTP, TLS, MSRP, or the selected codec/media stack.
- Security advisories, algorithm deprecation, certificate-policy changes, and dependency/license changes.
- Any newly observed Softil behavior that is not already explained by standards, profile, CeCoCo contract, or a server test.

Store a short decision record for each change: what changed, which implementation/tests are affected, whether backward compatibility is required, and who approved the profile update.

### 2.9.4 Minimum reading outputs before implementation

Before coding a feature, produce:

- a one-page scope and capability matrix;
- a standards traceability row or set of rows;
- a Stage 2-to-Stage 3 sequence diagram;
- a SIP/SDP/MCX message dictionary with mandatory/optional fields;
- a state-transition table including timeout, retry, duplicate, and release behavior;
- a security and secret-handling note;
- a sanitized positive trace and at least one negative test case;
- an explicit list of unresolved interpretations and vendor observations.

These outputs are the practical way to learn “how to build it like Softil” without copying Softil: learn the standard procedure, constrain it to the FRMCS/server profile, define the neutral API behavior, and prove it with traces and tests.

## 3. Current CeCoCo flow to preserve during migration

```text
CAM/CST configuration
        │
        ▼
McxEndpointSupervisorActor
  ├─ requests a generated MCX configuration
  ├─ initializes gateway JNI object
  ├─ sets license/configuration/logging
  ├─ configures shared audio buffers
  └─ creates one endpoint per configured identity
        │
        ▼
McxEndpointActor
  ├─ starts endpoint
  ├─ obtains authentication URL when required
  ├─ coordinates with cecoco-mcx-ext for authorization code
  ├─ submits code and waits for token event
  ├─ registers endpoint
  ├─ manages subscriptions/affiliation/aliases/settings
  └─ creates/removes session and conversation actors
        │
        ▼
McxSessionActor / McxConversationActor
  ├─ translates backend commands to JNI calls
  ├─ translates native callbacks to actor messages
  ├─ publishes call/floor/media/release events
  └─ owns shutdown behavior for a session
```

The registration documentation confirms a current sequence of configuration, native initialization, optional external authorization, token acquisition, SIP registration, and post-registration service setup. The independent implementation may simplify the internal sequence, but it must expose equivalent observable milestones to the Java actors until those actors are intentionally migrated.

The current `NONE_UUID` path can skip external authorization and use a preconfigured token. Support that only as an explicit configuration mode; never silently infer that credentials or tokens are valid.

## 4. Target architecture

### 4.1 Design rules

1. **Domain objects contain no JNI types.**
2. **Protocol objects contain no Akka or Java types.**
3. **The network/event-loop thread owns protocol state.**
4. **Application commands enter through a queue or serialized executor.**
5. **Callbacks leave the library as immutable event values.**
6. **No callback is invoked while holding a library mutex.**
7. **Every asynchronous operation has a correlation ID, timeout, terminal result, and cancellation rule.**
8. **Media buffers carry explicit length, timestamp, direction, format, and ownership.**
9. **Unknown protocol content is retained or rejected according to the applicable specification, never silently misinterpreted.**
10. **Vendor compatibility is isolated in one adapter, not spread through the stack.**

### 4.2 Proposed layers

```text
Public API / C ABI
        │
Client runtime and command executor
        │
Identity + authorization + registration
        │
MCX service procedures and domain models
        │
SIP/SDP transaction/dialog adapter
        │
RTP/SRTP and codec/media adapter
        │
Transport, timers, TLS, logging, metrics
```

Suggested responsibilities:

| Layer | Owns | Must not own |
|---|---|---|
| Public API | Commands, events, handles, errors, versioning | SIP message construction |
| Runtime | Threading, queues, timers, shutdown | Protocol semantics |
| Identity/registration | Identity state, token lifecycle, REGISTER/PUBLISH policy | Java actor behavior |
| MCX service | Calls, floor, affiliation, aliases, messaging, emergency | Socket details |
| SIP/SDP | SIP transactions/dialogs, headers, auth, offers/answers | CeCoCo call commands |
| RTP/media | Packets, codecs, jitter, media ports | Registration state |
| JNI adapter | Conversion and callback delivery | MCX business decisions |
| Mock/server harness | Deterministic simulated responses | Production protocol implementation |

### 4.3 Proposed repository structure

Start as a separate implementation directory or repository. Do not initially overwrite `src/softil` in place.

```text
independent-mcx-client/
├── CMakeLists.txt
├── cmake/
├── include/mcx/
│   ├── client.hpp              # lifecycle and factory
│   ├── types.hpp               # URI, service, identity, IDs
│   ├── commands.hpp            # application-to-client requests
│   ├── events.hpp              # client-to-application events
│   ├── calls.hpp               # call/session-facing types
│   ├── floor.hpp               # floor-facing types
│   ├── messaging.hpp           # conversation/message types
│   ├── media.hpp               # media port and frame contract
│   ├── config.hpp               # neutral configuration
│   └── errors.hpp               # stable categories and codes
├── src/
│   ├── api/
│   ├── runtime/                # executor, timers, shutdown
│   ├── identity/               # identities, tokens, registration
│   ├── authorization/           # authorization-code/token ports
│   ├── sip/                    # SIP adapter and transactions
│   ├── sdp/                    # offer/answer model and validation
│   ├── mcx/                    # MCX content and service procedures
│   ├── calls/                  # private/group call state machines
│   ├── floor/                  # floor-control state machine
│   ├── subscriptions/          # SUBSCRIBE/NOTIFY procedures
│   ├── affiliation/            # group affiliation procedures
│   ├── aliases/                # functional aliases
│   ├── messaging/              # conversations and message operations
│   ├── emergency/              # emergency alert procedures
│   ├── priority/               # priority and resource-priority policy
│   ├── rtp/                    # RTP/SRTP session manager
│   ├── codecs/                 # selected codec adapters
│   ├── media/                  # jitter, frame routing, ports
│   ├── security/               # TLS, token redaction, credential ports
│   ├── transport/              # UDP/TCP/TLS sockets
│   └── observability/          # logs, metrics, trace IDs
├── adapters/
│   ├── jni/                    # current Java compatibility layer
│   ├── c_api/                  # optional stable C ABI
│   ├── shared_memory_audio/    # current CeCoCo media bridge
│   └── sip_backend/            # selected third-party SIP library
├── mock/
│   ├── mock_client/
│   ├── mock_mcx_server/
│   └── scripted_scenarios/
├── tools/
│   ├── mcx_probe/
│   ├── packet_replay/
│   ├── trace_sanitizer/
│   └── config_validator/
├── tests/
│   ├── unit/
│   ├── contract/
│   ├── state_machine/
│   ├── protocol_vectors/
│   ├── integration/
│   ├── interop/
│   ├── soak/
│   └── fuzz/
├── testdata/
│   ├── sip/
│   ├── sdp/
│   ├── mcx/
│   └── rtp/
└── docs/
    ├── architecture.md
    ├── api-contract.md
    ├── protocol-traceability.md
    ├── configuration.md
    ├── security.md
    └── operations.md
```

## 5. Neutral API to define before protocol implementation

The API below is a design target, not a requirement to copy these exact names. Freeze it in an API contract document and contract tests before integrating Java.

### 5.1 Client lifecycle

```text
createClient(ClientConfig)
setEventSink(EventSink)
setAuthorizationProvider(AuthorizationProvider)
setMediaFactory(MediaFactory)
start()
stop()
```

Rules:

- `start()` validates configuration and starts the runtime; it does not imply registration.
- `stop()` is idempotent, cancels pending work, prevents new callbacks, and waits for owned resources to terminate.
- Destruction is only allowed after `stop()` completes.
- All methods return an immediate validation result or operation ID; network results are events.

### 5.2 Endpoint/identity API

```text
createEndpoint(EndpointConfig) -> EndpointHandle
startEndpoint(EndpointHandle)
stopEndpoint(EndpointHandle)
requestAuthorization(EndpointHandle, ServiceType)
submitAuthorizationCode(EndpointHandle, ServiceType, code)
setAccessToken(EndpointHandle, ServiceType, token, expiry)
registerEndpoint(EndpointHandle)
unregisterEndpoint(EndpointHandle)
getEndpointSnapshot(EndpointHandle)
```

The authorization provider should be a port. The library should support both:

- an external provider that returns an authorization code/token through CeCoCo services; and
- a preconfigured token mode for controlled environments.

Do not embed IDMS business logic in the MCX call engine unless requirements explicitly demand it.

### 5.3 Call/session API

```text
makeCall(EndpointHandle, CallRequest) -> OperationId / CallId
answerCall(CallId)
rejectCall(CallId, Reason)
hangUpCall(CallId, Reason)
changeCallPriority(CallId, PriorityRequest)
getCallSnapshot(CallId)
```

`CallRequest` should contain neutral fields:

```text
service_type
call_type
remote_party
local_party_override (optional)
local_alias (optional)
remote_alias (optional)
direction
media_mode: simplex | duplex
commencement_mode
floor_priority
implicit_floor_request
priority
alert_indicator
resource_priority
adhoc_criteria (optional)
adhoc_members (optional)
calling_user_id (optional)
```

### 5.4 Floor API

```text
requestFloor(CallId, FloorRequest)
releaseFloor(CallId)
getFloorSnapshot(CallId)
```

The library must prevent ordinary media transmission when the service requires floor ownership and the endpoint does not own the floor. The API must represent queued, denied, revoked, pre-empted, and expired floor requests rather than reducing them all to “PTT failed”.

### 5.5 Services API

```text
subscribeAffiliation(EndpointHandle, ServiceType, enabled)
affiliate(EndpointHandle, ServiceType, groups)
deaffiliate(EndpointHandle, ServiceType, operationId)
subscribeFunctionalAliases(EndpointHandle, ServiceType, enabled)
addFunctionalAliases(EndpointHandle, ServiceType, aliases)
removeFunctionalAliases(EndpointHandle, ServiceType, operationId)
setSettings(EndpointHandle, ServiceType, enabled)
requestSettings(EndpointHandle, enabled)
sendEmergencyAlert(EndpointHandle, EmergencyAlertRequest)
```

Every service operation must return an operation ID and eventually one terminal event containing success/failure and server reason data where available.

### 5.6 Conversation/messaging API

```text
createConversation(EndpointHandle, ConversationRequest) -> ConversationId
terminateConversation(ConversationId)
sendMessage(ConversationId, MessageRequest) -> MessageId
cancelMessage(MessageId)
```

Use a generic message envelope with typed payloads for text, binary, status, location, file URL, hyperlink, and attachments only when the target requirements confirm them. Do not expose vendor handles, native pointers, or raw SDK callback structures.

### 5.7 Events

At minimum define immutable events for:

```text
endpoint_state_changed
authorization_state_changed
registration_state_changed
subscription_state_changed
affiliation_operation_completed
affiliation_information_received
functional_alias_operation_completed
functional_alias_information_received
settings_operation_completed
incoming_call
call_state_changed
call_info_changed
rx_state_changed
tx_state_changed
floor_state_changed
call_priority_changed
media_state_changed
conversation_created
conversation_removed
message_state_changed
incoming_message
emergency_alert_received
network_failure
security_failure
protocol_error
operation_failed
```

For every event document:

- event thread/dispatcher;
- endpoint/call/conversation correlation IDs;
- ordering guarantees;
- whether events can be duplicated;
- whether events can arrive after a terminal event;
- object ownership and snapshot lifetime;
- shutdown behavior.

## 6. State machines to implement explicitly

Do not implement the new client as a collection of callbacks with implicit state. Write transition tables and tests first.

### 6.1 Runtime state

```text
NEW → STARTING → RUNNING → STOPPING → STOPPED
                 └────────→ FAILED
```

Reject commands in `NEW`, `STOPPING`, and `STOPPED` with stable errors. `stop()` must be safe from every state.

### 6.2 Authorization and registration

Maintain service-specific state because MCPTT, MCDATA, and MCVIDEO may not be enabled or authorized together:

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
  → REGISTRATION_FAILED
  → RECONNECTING
```

Required cases:

- external authorization code ACK delayed or duplicated;
- code rejected or expired;
- token rejected or expired;
- preconfigured-token mode;
- REGISTER with token versus token publication policy;
- registration refresh;
- 401/403 and transport failure;
- server restart;
- interface/address change;
- explicit unregister;
- stop while any request is in flight.

The current Java endpoint exposes intermediate authentication-code ACK and token-response states. Preserve them in the compatibility adapter even if the new internal model uses more precise states.

### 6.3 Call/session

Outgoing path:

```text
IDLE → REQUESTED → INVITE_SENT → PROCEEDING → OFFERING
     → CONNECTED → RELEASING → DISCONNECTED
```

Incoming path:

```text
INCOMING_RECEIVED → PRESENTED → ANSWERING → CONNECTED
                  → REJECTING → DISCONNECTED
```

Define responses for:

- busy, forbidden, unauthorized, unavailable, timeout;
- cancellation before answer;
- remote release;
- duplicate/retransmitted SIP messages;
- media negotiation failure;
- invalid MCX content;
- local release during every state;
- network loss while connected;
- late callbacks after session destruction.

### 6.4 Floor control

```text
NO_FLOOR → REQUESTING → QUEUED → GRANTED → TALKING
                         └──────→ DENIED
GRANTED/TALKING → RELEASING → NO_FLOOR
GRANTED/TALKING → REVOKED   → NO_FLOOR
```

Cover priority pre-emption, simultaneous requests, duplicate messages, stale transactions, server denial, release while audio is queued, and emergency behavior.

### 6.5 Media

```text
NO_MEDIA → NEGOTIATING → READY → PLAYING/CAPTURING
                         └─────→ FAILED
READY/PLAYING/CAPTURING → MUTED → READY
READY/PLAYING/CAPTURING → STOPPING → NO_MEDIA
```

Media state must be derived from both SDP/RTP state and call/floor policy. A connected SIP dialog is not proof that audio is usable.

## 7. The first implementation cases

Implement these cases in this order. Each case must be executable, observable, and tested before starting the next.

### Case 0 — Mock lifecycle and contract

**Purpose:** prove the API, object ownership, callback routing, and Java integration without any MCX protocol.

Implement:

1. Create client and endpoint.
2. Start/stop client and endpoint.
3. Emit deterministic registration events.
4. Create/remove a session.
5. Emit call state, RX/TX, priority, and release events.
6. Attach/detach a test media port.
7. Verify duplicate commands and shutdown behavior.

Success criteria:

- Java gateway starts with the new library/mock and no Softil library loaded.
- All native objects are released exactly once.
- Callback events arrive on the expected actor path.
- The mock can inject delayed, duplicate, rejected, and out-of-order events.

Use the existing `jni_mock` behavior as a starting behavioral reference, but correct its limitations and do not treat it as a production implementation.

### Case 1 — Standalone transport and SIP probe

Before JNI, build `mcx_probe` that can:

- bind configured signaling and RTP addresses;
- use the selected UDP/TCP/TLS backend;
- log sanitized SIP transactions with correlation IDs;
- parse and serialize the required headers;
- handle authentication challenges;
- parse SDP offers/answers;
- capture a reproducible trace.

The probe must first be tested against a local scripted SIP peer. Only then connect it to the MCX Server.

### Case 2 — Registration with preconfigured token

Start with the least ambiguous authorization path:

1. Load a sanitized endpoint configuration.
2. Load a token through an injected credential provider.
3. Build and send the required registration sequence.
4. Parse success/failure and expiry.
5. Refresh registration.
6. Unregister cleanly.

Do not start with OAuth authorization-code exchange inside the client. The current CeCoCo external service already coordinates this flow; first prove that the new client can consume a token and register.

### Case 3 — Outgoing private call without audio

This is the first real vertical slice:

```text
Java makeCall
  → JNI adapter
  → neutral CallRequest
  → SIP/MCX call state machine
  → MCX Server
  → call events
  → Java McxSessionActor
```

Implement:

- private-call request;
- required SIP/SDP/MCX content;
- proceeding/offering/connected events;
- answer/accept behavior as required by the server;
- hangup and terminal release;
- timeout and error mapping;
- stable call ID correlation.

At this point it is acceptable for media to be disabled, but the SDP must be valid and the call state must be complete.

### Case 4 — Incoming private call without audio

Implement:

- incoming call detection;
- creation of a Java-compatible session object;
- injection of demanded call information;
- incoming call presentation;
- answer, reject, and release;
- remote release and timeout;
- cleanup if the actor dies before the call terminates.

Verify that no native callback accesses a destroyed Java object.

### Case 5 — Audio and media bridge

Add audio only after Cases 3 and 4 work without media.

1. Implement in-process `MediaPort` tests with generated audio frames.
2. Add RTP packetization/depacketization and required codec(s).
3. Validate SDP payload types, clock rates, packetization, and direction.
4. Add jitter and loss statistics.
5. Implement the shared-memory compatibility adapter.
6. Connect player/recorder IDs used by `JniMcxSession`.
7. Test release and failure while frames are queued.

The media contract should define frame size, sample rate, channel count, timestamp, ownership, backpressure, mute behavior, and what happens when no floor is owned.

### Case 6 — PTT/floor control for the first supported group call

Only after private-call media is stable:

- create a group call;
- implement request/grant/deny/release/revoke;
- map floor events to Java RX/TX events;
- ensure local capture is coupled to floor ownership;
- preserve transmitting party and functional alias information;
- test priority and collision cases.

Do not assume private-call floor behavior. The indexed FRMCS material includes profiles where floor control for private calls is not supported; the target MCX Server profile must decide which procedures apply.

### Case 7 — Group, pre-established, broadcast, listening, and ad-hoc variants

Add call types one by one, only when a real use case and protocol trace exist. For each type define:

- request URI and identity rules;
- commencement/answer rules;
- media mode;
- floor behavior;
- late-entry behavior;
- release reason mapping;
- CeCoCo display/remote-party mapping;
- ad-hoc range allocation if required.

The current gateway has an incoming ad-hoc range and formatter. Implement that as a dedicated policy service, not inside the SIP parser.

### Case 8 — Subscriptions, affiliation, aliases, and settings

Implement one operation family at a time:

1. SUBSCRIBE/NOTIFY transport and expiration.
2. affiliation publish/request and operation correlation.
3. functional-alias publish/request and operation correlation.
4. settings activation/request.
5. incoming XML/content validation and snapshot updates.

The public API should return semantic results; raw XML may be retained for diagnostics but should not be the primary domain model.

### Case 9 — Messaging/conversations

Implement conversation lifecycle before adding every message type:

- create local/private/group conversation;
- send text message;
- receive text message;
- message state changes and expiry;
- terminate conversation;
- then binary/status/location and other types if required.

The current wrappers distinguish conversations from individual message operations and track message expiry. Preserve that behavior conceptually without exposing vendor handles.

### Case 10 — Priority and emergency

Implement after ordinary call and floor flows are reliable:

- normal, imminent-peril, and emergency priority;
- alert indicator;
- resource-priority value;
- priority change during an active call;
- emergency alert enable/cancel;
- normal versus ad-hoc emergency;
- criteria, group ID, functional alias, location, user-requested priority, and originator data;
- received-indicator flags.

Emergency behavior requires dedicated standards traceability, negative tests, authorization review, and test-server approval. It must not be enabled accidentally by a malformed or missing field.

## 8. First execution plan

### Before writing protocol code

Complete these documents:

- `docs/scope-matrix.md`: supported services, call types, codecs, transports, and explicit non-goals.
- `docs/current-contract.md`: Java/JNI methods, events, states, error mappings, and lifecycle.
- `docs/softil-responsibility-inventory.md`: wrapper responsibility mapped to neutral module, without copied implementation.
- `docs/protocol-traceability.md`: requirement/specification/procedure/trace/test mapping.
- `docs/dependency-and-license-inventory.md`: selected open-source dependencies and licenses.
- `docs/configuration.md`: neutral schema and secret-handling rules.

### First two weeks, in order

1. Freeze the Java/JNI contract with tests.
2. Create the standalone CMake project and public headers.
3. Implement runtime, serialized command executor, timers, and deterministic shutdown.
4. Implement `MockMcxClient` and scripted event injection.
5. Build the JNI adapter against the mock.
6. Run the Java gateway’s endpoint/session lifecycle against the mock.
7. Build the standalone SIP probe.
8. Add packet trace sanitization and replay tools.
9. Select SIP/SDP/RTP/TLS dependencies after a capability/license review.
10. Implement preconfigured-token registration.
11. Begin the outgoing private-call no-audio vertical slice.

Do not begin group calls, emergency alerts, or full messaging before the private-call path has a repeatable trace and automated failure tests.

## 9. Mapping from the current Softil wrapper to the new design

| Current wrapper responsibility | Neutral replacement module | Migration note |
|---|---|---|
| `ClientMcx` singleton, config, stores, callbacks | `McxClientRuntime` | Remove global singleton from the domain layer; inject dependencies |
| `IdentityStore` | `EndpointRegistry` | Own endpoint handles and snapshots |
| `McxIdentity` / `McxIdentityInfo` | `Endpoint`, `ServiceIdentity`, `RegistrationSession` | One explicit state machine per service identity |
| `SipIdentity` register/unregister | `SipRegistrar` + registration FSM | Keep SIP details below identity service |
| OAuth/auth-code bridge | `AuthorizationProvider` port | External CeCoCo service remains an implementation |
| `CallStore` | `SessionRegistry` | Call IDs and lifetime are library-owned |
| `SipCall` / `McxDemandedCall` | `CallSession` | Split signaling, service policy, floor, and media state |
| `McxPrivateCall` / `McxGroupCall` | call-type procedure implementations | Share common session infrastructure; do not duplicate lifecycle |
| Softil `CallProxy` callbacks | event sink / adapter events | Immutable event values and explicit correlation |
| `SubscribeMcxOperation` | `SubscriptionTransaction` | Generic SIP transaction plus typed MCX content |
| `AffiliationOperation` | `AffiliationTransaction` | Operation IDs and group snapshot |
| `FunctionalAliasOperation` | `FunctionalAliasTransaction` | Operation IDs and alias snapshot |
| `SettingsOperation` | `SettingsTransaction` | Typed settings state |
| `EmergencyAlertOperation` | `EmergencyAlertProcedure` | Dedicated validation and authorization |
| `MessageStore` | `ConversationRegistry` | No vendor handles |
| `MessageConversation` | `ConversationSession` | Conversation lifecycle and expiry |
| `CircularBufferDevice` | `MediaPort` | Shared-memory implementation belongs in an adapter |
| `FrameBufferPool`/`CircularBuffer` | `FrameQueue`/`SharedMemoryMediaPort` | Make ownership and backpressure explicit |
| Softil configuration INI | neutral config + server-profile adapter | Never expose vendor-only keys in public API |
| Softil log callback | structured `LogSink` | Redact credentials, tokens, and sensitive URIs |

## 10. JNI migration strategy

Use a strangler migration rather than rewriting Java and native code simultaneously.

### Stage A — compatibility harness

- Keep the current Java classes and actor protocol unchanged.
- Implement a new native backend that satisfies the existing JNI base methods.
- Link it without any Softil include or library directory.
- Run all Java endpoint/session tests against the mock backend.

### Stage B — neutral native backend behind the same JNI adapter

- Replace mock operations with the independent runtime.
- Keep conversions from Java beans to neutral request types in one adapter file.
- Keep conversions from neutral events to Java callback calls in one adapter file.
- Preserve call ID, session creation, demanded-call information, RX/TX, priority, and terminal event behavior.

### Stage C — optional Java API cleanup

Only after interoperability is proven:

- replace vendor-derived names and numeric constants in Java;
- introduce clearer endpoint/service/call snapshots;
- remove compatibility methods that are no longer needed;
- update actor documentation and tests.

### JNI safety requirements

- Attach native threads to the JVM through one managed utility.
- Hold global references only while the Java object is valid.
- Never call Java while holding a protocol or registry lock.
- Convert Java strings with explicit lifetime and encoding rules.
- Validate all array lengths and null values.
- Cancel callbacks before deleting native objects.
- Make `destroySession`, `destroyConversation`, endpoint stop, and gateway stop idempotent.
- Test actor termination during active signaling, media, and timers.

## 11. Dependency selection

Evaluate dependencies before implementation, using a written scorecard:

### SIP/SDP

Required checks:

- UDP, TCP, TLS;
- transaction and dialog behavior;
- Digest/AKA or other authentication required by the target profile;
- custom headers and content types;
- reliable provisional responses if required;
- re-INVITE/UPDATE/session timers;
- IPv4/IPv6 behavior;
- external event-loop integration;
- license and redistribution terms.

### RTP/media

Required checks:

- RTP/RTCP and SRTP support;
- payload types and packetization required by the MCX Server;
- AMR-NB/AMR-WB/EVS or the actual selected codec profile;
- jitter and packet-loss reporting;
- zero-copy or bounded-copy integration with the CeCoCo media path;
- clean shutdown and thread ownership.

### Security and XML/content

Required checks:

- TLS certificate and hostname validation;
- token/credential provider separation;
- XML/content parser hardening if XML remains in the server profile;
- bounded body sizes;
- dependency CVE response process;
- reproducible/pinned builds.

Do not reuse the current `third_parties` vendor libraries merely because they are already present in the repository. The independent build must prove that no Softil/BeeHD binary is required.

## 12. Testing strategy

### 12.1 Contract tests

Run the same contract suite against:

- current Softil backend, where legally and operationally available;
- existing mock;
- new independent mock;
- new independent real backend.

Contract tests must cover method results, event order, IDs, state transitions, demanded-call information, priority mapping, media attach/detach, and shutdown.

### 12.2 Unit tests

Test:

- URI and identity normalization;
- configuration validation and secret redaction;
- SIP/SDP parse/serialize behavior used by the project;
- MCX content validation;
- timer/retry calculations;
- registration, call, floor, subscription, affiliation, alias, messaging, emergency, and priority transitions;
- RTP sequence/timestamp and jitter behavior;
- operation correlation and expiry;
- JNI conversion helpers.

### 12.3 Protocol vectors and replay

For every supported scenario, store a sanitized test vector containing:

- scenario and software versions;
- endpoint/server profile;
- expected call or operation result;
- SIP messages with secrets removed;
- SDP body;
- MCX content with identifiers minimized/anonymized;
- RTP metadata or short synthetic media sample;
- expected state/event timeline.

Validate protocol properties rather than byte-for-byte equality unless the specification requires exact encoding.

### 12.4 Fuzz and negative tests

Fuzz SIP, SDP, MCX content, RTP packets, configuration, and JNI input. Verify:

- no crash or unbounded allocation;
- no invalid state transition;
- no callback after destruction;
- malformed unknown content is handled according to policy;
- duplicate/late/reordered messages are safe;
- all network-sourced lengths are bounded.

### 12.5 Interoperability and soak tests

Against the controlled MCX Server, test:

- registration and refresh;
- auth/token failure;
- outgoing and incoming private calls;
- group calls and floor control;
- codecs and RTP;
- subscriptions, affiliation, aliases, settings;
- priority and emergency if in scope;
- server restart and network loss;
- multiple identities and simultaneous calls;
- multi-hour calls and multi-day gateway soak.

Collect protocol traces, Java events, native structured logs, CPU, memory, queue depth, RTP statistics, and recovery time.

## 13. Configuration and deployment

### 13.1 Neutral configuration sections

Use a versioned schema similar to:

```text
runtime
  worker model, queue limits, shutdown timeout
endpoint
  identity URI, service types, display name, client ID
server
  registrar/proxy/service URIs, transport, ports
authorization
  provider mode, token path/provider, refresh policy
registration
  expiry, retry, keepalive, timeout policy
media
  RTP range, codecs, sample rate, packetization, jitter
security
  CA, certificate, private key reference, verification policy
features
  call types, floor, aliases, affiliation, messaging, emergency
compatibility
  Java/JNI version, shared-memory layout, legacy mappings
observability
  log level, metrics, sanitized trace options
```

Do not expose secrets as ordinary strings in logs or diagnostics. Prefer a credential-provider interface or protected file descriptor/path and redact values centrally.

### 13.2 Linux/CMake integration

The current target is Linux/WSL-oriented C++ with CMake and JNI. The independent build should provide:

- a standalone `libmcxclient` target with no JNI dependency;
- a separate `libmcxclient-jni` adapter target;
- a mock target with no network/vendor dependency;
- a protocol probe executable;
- unit and integration test targets;
- sanitizer and coverage presets;
- a dependency manifest and license report;
- installation rules for headers, shared libraries, and configuration examples.

Only the JNI target should require JDK headers. The core client must compile and test without Java.

### 13.3 Packaging checklist

Before deployment, define:

- shared-library name and ABI policy;
- JNI loading path;
- runtime dependency list;
- systemd startup/shutdown order;
- certificate/key ownership and permissions;
- configuration migration;
- health/readiness checks;
- core dump and crash diagnostics;
- upgrade and rollback path;
- whether the old Softil package can remain installed but unused during pilot.

## 14. Phases and exit criteria

### Phase 0 — scope and evidence

Deliverables:

- approved scope matrix and non-goals;
- current JNI/Java contract;
- wrapper-responsibility inventory;
- sanitized traces;
- server test access;
- requirements traceability;
- dependency/IP/license review.

Exit when the team can identify the exact first call type, service type, transport, codec, authorization mode, and media path to support.

### Phase 1 — core runtime and mock

Deliverables:

- neutral API;
- runtime/queue/timer/shutdown;
- deterministic mock;
- JNI compatibility adapter;
- lifecycle and callback contract tests.

Exit when the Java gateway passes endpoint/session lifecycle tests without Softil.

### Phase 2 — SIP/SDP foundation

Deliverables:

- selected SIP backend;
- transport and TLS;
- transaction/dialog correlation;
- authentication;
- SDP model and validation;
- standalone probe.

Exit when the probe completes the required controlled SIP flow and produces diagnosable traces.

### Phase 3 — registration and private-call signaling

Deliverables:

- token-provider port;
- service identity and registration FSM;
- outgoing/incoming private call FSM;
- release/error mapping;
- Java events and session lifecycle.

Exit when incoming and outgoing private calls work without audio against the target server profile.

### Phase 4 — RTP and audio bridge

Deliverables:

- RTP/SRTP session manager;
- required codec(s);
- media port contract;
- shared-memory compatibility adapter;
- media metrics and failure handling.

Exit when bidirectional audio works and cleanly stops on release, timeout, network failure, and actor shutdown.

### Phase 5 — floor and group calls

Deliverables:

- group-call procedures;
- floor FSM;
- PTT/RX/TX mapping;
- queue, deny, revoke, priority, and collision handling;
- late-entry behavior.

Exit when floor and media behavior are deterministic under concurrent and failure cases.

### Phase 6 — product services

Add only confirmed requirements:

- affiliation;
- functional aliases;
- settings;
- messaging/conversations;
- priority changes;
- emergency alerts;
- MCData/MCVideo;
- special FRMCS procedures.

Each feature exits only with requirement mapping, state tests, negative tests, server trace, and Java compatibility evidence.

### Phase 7 — resilience and production hardening

Deliverables:

- re-registration and token refresh;
- server/network recovery;
- resource and backpressure limits;
- metrics/health checks;
- fuzz/sanitizer/soak results;
- packaging and operational runbook.

Exit when failure-injection, soak, security, and operational acceptance tests pass.

### Phase 8 — migration and removal

1. Keep backend selection configurable.
2. Run identical contract and interoperability suites against old and new implementations.
3. Compare state/event timelines, protocol traces, media, timing, and failure behavior.
4. Deploy to a non-production environment.
5. Run a controlled pilot with rollback available.
6. Remove Softil only after product, interoperability, security, licensing, and operations sign-off.

## 15. Definition of done

The library is ready to replace Softil for an agreed feature set when all of the following are true:

- the core library builds without Softil headers, binaries, or runtime libraries;
- the JNI adapter is the only Java-specific boundary;
- the supported feature set is explicitly documented;
- requirements are traced to implementation and tests;
- registration, call, floor, media, service, and recovery state machines have transition tests;
- incoming malformed/duplicate/late protocol events are safe;
- Java actor event ordering and call IDs are compatible;
- audio works through the production media bridge for supported calls;
- tokens, credentials, keys, and sensitive traces are protected;
- sanitizers, fuzz tests, soak tests, and dependency scans have passed;
- interoperability has been demonstrated against the target MCX Server profile;
- packaging, monitoring, rollback, and incident diagnosis are documented;
- legal/IP and dependency-license review is complete;
- the old implementation can be restored during migration.

## 16. Main risks and mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| Treating SIP as MCX | Basic SIP calls work but MCX interoperability fails | Separate SIP, MCX service, floor, media, and security requirements |
| Scope explosion | No meaningful completion point | Approve a narrow MVP and add one call/service case at a time |
| Hidden vendor behavior | Intermittent incompatibilities | Use source inventory, sanitized traces, specifications, and server tests |
| No server access | Protocol work cannot be verified | Secure a dedicated MCX Server test profile in Phase 0 |
| Recreating vendor lock-in | New code becomes opaque and hard to replace | Keep neutral API and protocol layers independent |
| JNI lifetime races | Crashes and corrupt callbacks | Thin adapter, serialized ownership, global-reference tests, sanitizers |
| Codec/media mismatch | Signaling succeeds but no audio | Implement media as an early vertical slice after no-audio calls |
| Incorrect floor semantics | PTT collisions or unauthorized transmission | Explicit floor FSM and race-condition tests |
| Emergency behavior assumed | Safety/product failure | Dedicated requirements, authorization, traces, and negative tests |
| Secrets in configuration/traces | Security incident | Rotate exposed values, sanitize archives, central redaction, protected providers |
| Dependency/license problems | Cannot ship the library | Pinned dependency/license inventory before adoption |
| Weak recovery behavior | Gateway remains unavailable after server/network failure | Failure injection, re-registration, token refresh, and soak testing |

## 17. Immediate next action list

Start with these concrete artifacts and do not skip ahead:

1. Freeze `docs/current-contract.md` from the current Java/JNI classes.
2. Add a no-Softil build target for the independent core library.
3. Define `EndpointSnapshot`, `CallRequest`, `CallSnapshot`, `Event`, `MediaFrame`, and `Error` in public headers.
4. Implement the runtime and deterministic mock.
5. Connect the existing Java gateway to the mock and record the event timeline.
6. Create the feature/scope matrix with the product owner and MCX Server owner.
7. Obtain one sanitized registration trace and one sanitized private-call trace.
8. Build the standalone SIP/SDP probe.
9. Select the SIP/RTP/TLS dependencies using the capability and license scorecard.
10. Implement token-injection registration, then the outgoing private-call no-audio case.

The first success milestone is deliberately small:

```text
Independent core library builds without Softil
        +
Java gateway runs against the independent mock
        +
Standalone probe registers with the target server
        +
Independent client completes one private call without audio
        +
The call is released cleanly and all events are observed in Java
```

Everything else should be built on that verified foundation.
