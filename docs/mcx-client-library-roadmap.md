# Roadmap: MCX client library for gateway integration

## 1. Purpose and scope

This project is a native C++ MCX client library intended to be consumed by a
separate gateway application. The library is responsible for communicating
with the IMS and MCX server-side systems required by the selected deployment
profile. The gateway is the library's consumer; implementing the gateway is
outside this repository's scope.

The intended boundary is:

```text
External gateway
      │
      │ stable C++ API (or a separately agreed C ABI)
      ▼
MCX client library (this project)
  ├── client lifecycle and endpoint state
  ├── authorization integration, when required
  ├── SIP/IMS signaling through a SIP backend such as PJSIP
  ├── MCX service procedures and state machines
  └── media transport and processing, as features require
      │
      ├── IMS/SIP infrastructure
      └── MCX application server
```

This library does not implement a gateway, Java/JNI integration, an Asterisk
connector, or a replacement for a named commercial SDK. Do not add those
systems to the design unless the library's consuming application explicitly
requires an adapter in a separate target.

The repository currently contains configuration models and INI loading,
logging, a console mock application, and a configuration test. It does **not**
yet contain PJSIP integration, network signaling, MCX service procedures,
registration, or media handling. This roadmap starts from that state.

## 2. Responsibilities and boundaries

### This library owns

- A documented API that a gateway can use without depending on SIP internals.
- Client and endpoint lifecycle, state, asynchronous operation results, and
  event delivery.
- Configuration validation and safe handling of credentials and tokens.
- IMS registration/signaling integration through the selected SIP backend.
- MCX-specific signaling content, service procedures, and state transitions
  required by the agreed feature scope.
- Media integration only for features that need it and only after the target
  profile and gateway media contract are known.
- Deterministic shutdown, timeouts, error reporting, and recovery behavior.

### The consuming gateway owns

- Its own application lifecycle, user/business logic, and external interfaces.
- Mapping gateway operations to the library's public API.
- Any adapter needed to expose the library through a gateway-specific ABI.
- Providing credentials/tokens through the agreed secure interface.

The library must not depend on gateway classes, frameworks, source code, or
runtime libraries. Keep gateway-specific adapters outside the protocol core.

### Protocol boundary

PJSIP is a candidate SIP stack for SIP transactions, dialogs, transports, and
related SIP facilities. It does not implement MCX application procedures for
the library. MCX-specific procedures, content, authorization policy, and
service state machines remain the responsibility of this project.

IDMS/OAuth, if required by the deployment, is a distinct authorization
integration. Do not assume PJSIP performs that flow. Define an authorization
interface and establish whether the gateway or library obtains and refreshes
tokens before implementing it.

## 3. Current repository baseline

The existing code provides:

- `ConfigBase`, `ConfigManager`, and IDMS, SIP, and MCX configuration models.
- INI parsing and typed value access through `IniHelper`.
- A `Logger` with a shared minimum log level.
- A CMake library target, a mock console executable, and an INI test.

The config objects currently describe values; they do not establish network
connections. The first implementation work should preserve the usable config
and test foundation while introducing a separate client API and networking
layer. Avoid placing PJSIP calls inside config classes or `ConfigManager`.

## 4. Decisions required before implementing server procedures

Record these decisions in a project profile document before depending on
server-specific behavior:

1. **Initial use case:** for example, IMS registration, MCPTT registration,
   outgoing private call, incoming call, or group/PTT call.
2. **Server topology:** which IMS/SIP registrar, proxy, and MCX application
   server endpoints the client must contact, and how those roles relate.
3. **Identity and authorization:** IMPI/IMPU or other identities, AKA/Digest
   requirements, token source, token use, expiry, and refresh behavior.
4. **Transport/security:** UDP, TCP, TLS, certificate policy, IPv4/IPv6, and
   any deployment-specific network constraints.
5. **MCX profile:** supported service types, procedures, SIP headers/content,
   and the exact standards/profile release used by the target server.
6. **Media:** whether the first milestone is signaling-only; if media is in
   scope, required codecs, RTP/SRTP profile, packetization, and gateway media
   interface.
7. **Consumer interface:** C++ ABI compatibility needs, whether a C ABI is
   required, threading/callback expectations, and ownership/lifetime rules.
8. **Build/deployment platforms:** development toolchains and actual target OS,
   compiler, architecture, and packaging constraints.
9. **Interoperability access:** a local scripted SIP peer and a controlled
   IMS/MCX server profile, plus sanitized traces or equivalent test evidence.

Do not infer required behavior solely from the current INI keys or enum values.
Treat them as existing implementation details to review against the agreed
profile.

## 5. Target architecture

Keep protocol behavior behind small interfaces so the library can be tested
without a live network:

```text
Public MCX client API
        │
Client runtime, operation queue, timers, event dispatcher
        │
Identity / authorization / registration
        │
MCX service procedures and explicit state machines
        │
SIP/SDP adapter ───── PJSIP (candidate SIP backend)
        │
Transport and security
        │
RTP/SRTP and media adapter (when in scope)
```

Suggested source organization as the project grows:

```text
include/mcx/             Public client API and neutral value types
include/mcx/config/      Public configuration types
src/client/              Lifecycle, operations, and event delivery
src/identity/            Identity and authorization coordination
src/sip/                 SIP backend interface and PJSIP adapter
src/mcx/                 MCX procedures, content, and state machines
src/media/               Media interfaces and implementation, when needed
apps/mock_app/           Manual consumer/example application
tests/                   Unit, contract, and integration tests
docs/                    Profile, API, and protocol decisions
```

This is a direction, not a requirement to create every directory immediately.
Keep the current repository small until a feature needs a new component.

### Design rules

- Public API types must not expose PJSIP types or handles.
- Configuration parsing must not perform network operations.
- Application calls enter a serialized execution context, or have explicitly
  documented thread-safety guarantees.
- Deliver asynchronous network results as documented events or operation
  results; define callback thread and shutdown behavior.
- Correlate operations and events with stable endpoint/call/operation IDs.
- Make stop/disconnect behavior deterministic and safe when operations are
  pending.
- Redact passwords, tokens, private keys, and sensitive message content from
  logs and diagnostic output.
- Treat the target standards and server profile—not packet examples alone—as
  the source of protocol requirements.

## 6. Public API to define

Freeze the first API contract and test it before it becomes coupled to PJSIP.
Names below are illustrative:

```text
createClient(ClientConfig, EventHandler)
start()
stop()
createEndpoint(EndpointConfig) -> EndpointId
registerEndpoint(EndpointId)
unregisterEndpoint(EndpointId)
getEndpointState(EndpointId)
```

Later operations can be added only for agreed use cases:

```text
makeCall(EndpointId, CallRequest) -> OperationId / CallId
answerCall(CallId)
rejectCall(CallId, Reason)
hangUpCall(CallId, Reason)
requestFloor(CallId)
releaseFloor(CallId)
```

Document at minimum:

- What each synchronous return means versus the later network result.
- Valid state transitions and errors for invalid commands.
- Callback/event execution thread, ordering, and lifetime.
- Whether calls can be concurrent and what limits apply.
- Ownership of configuration, credential providers, media buffers, and events.
- Shutdown, cancellation, timeout, and reconnection semantics.

Avoid exposing vendor or PJSIP implementation types through this API. Keep
gateway-specific bindings in a separate adapter only if the consuming gateway
requires one.

## 7. Implementation milestones

### Milestone 0 — scope and profile

Deliver:

- A narrow MVP/use-case statement.
- The decisions listed in Section 4.
- A protocol traceability record connecting each required behavior to the
  applicable standard/profile and test.
- A sanitized configuration example with no real credentials.

Exit when the target server owners and library consumer agree on the first
end-to-end use case and the environment needed to test it.

### Milestone 1 — client contract and deterministic mock

Implement the public lifecycle and endpoint API without networking. Add a
mock backend or fake transport that can emit deterministic success, failure,
timeout, and disconnect events.

Test:

- Start/stop and repeated shutdown.
- Invalid command/state handling.
- Event order and operation correlation.
- Callback behavior during shutdown.
- Configuration validation and credential redaction.

The mock is a test tool for the library consumer contract; it is not a gateway
implementation or proof of MCX interoperability.

### Milestone 2 — PJSIP build and standalone SIP probe

Evaluate and pin a PJSIP version, license, platform support, build approach,
and required features before integrating it into the public library target.
Keep PJSIP behind an internal adapter.

Build a small probe or integration test that can:

- Initialize and shut down the PJSIP runtime safely.
- Configure the agreed signaling transport.
- Send and receive a simple SIP request with a local scripted SIP peer.
- Exercise transaction timeouts, errors, and clean shutdown.
- Capture sanitized, correlated SIP traces.

Do not yet claim MCX support based on successful generic SIP messaging.

### Milestone 3 — IMS connectivity and registration

Implement only the registration/authentication sequence required by the
agreed IMS and MCX server profile:

1. Load and validate endpoint configuration.
2. Obtain credentials or token using the agreed provider boundary.
3. Establish the selected SIP transport securely.
4. Perform registration and interpret success, expiry, and failure.
5. Refresh/re-register and unregister according to the profile.
6. Test network loss, server rejection, expiry, and shutdown during a request.

First prove the flow against a local scripted peer; then verify it against the
controlled IMS/MCX environment.

### Milestone 4 — first MCX signaling use case

Implement one selected MCX procedure end to end. If the agreed MVP is a
private call, start with signaling without media:

- Construct the required SIP/MCX request and content.
- Correlate responses to a stable call ID.
- Model incoming, proceeding, connected, rejected, failed, and released states.
- Handle cancellation, timeout, duplicate/late messages, and remote release.
- Validate behavior against the target server and automated traces.

Do not start group, emergency, or messaging features until this slice is
repeatable and covered by negative tests.

### Milestone 5 — media and floor control, if required

After signaling works, add media only to the calls in the agreed scope:

- Confirm RTP/SRTP, codec, packetization, and SDP requirements.
- Define a media-port interface independent of the gateway implementation.
- Test generated media locally before connecting a real gateway media path.
- Couple transmit behavior to floor ownership where the MCX procedure requires
  it.
- Test media shutdown on release, timeout, and network failure.

Implement floor/PTT and group-call behavior as explicit state machines based
on the applicable MCX profile, not as generic SIP call behavior.

### Milestone 6 — additional services and production readiness

Add only confirmed requirements, one feature family at a time: group calls,
affiliation, aliases, subscriptions, messaging, emergency, priority, MCData,
or MCVideo.

For each feature require:

- A profile/standards traceability entry.
- Success and failure state-transition tests.
- Sanitized server interoperability evidence.
- Secret-safe logging and bounded parsing.
- Documented consumer API behavior.

Complete recovery, resource limits, dependency updates, packaging, security
review, and soak testing before production use.

## 8. Standards and interoperability process

Create a controlled profile document recording the exact editions and
requirements used. The following are candidate references to check against
the target product and server profile; this list is not a claim that every
feature is required:

- MCX common architecture: 3GPP TS 22.280 and TS 23.280.
- MCPTT requirements, architecture, and protocol: TS 22.179, TS 23.379, and
  TS 24.379.
- MCData, if in scope: TS 22.282, TS 23.282, and TS 24.282.
- MCVideo, if in scope: TS 22.281, TS 23.281, and TS 24.281.
- MCX security: TS 33.180 and the applicable IMS security profile.
- IMS/SIP: TS 24.229 and the applicable SIP/SDP RFCs.
- RTP/SRTP and codecs: the specific RFCs and 3GPP codec/profile documents
  selected by the target deployment.

For each requirement, record:

```text
Feature/use case
Applicable standard/profile and release
Required procedure and messages
Server capability or configuration dependency
Implementation module
Positive/negative test
Interoperability evidence
```

Treat sanitized traces as evidence of a particular server behavior, not as a
replacement for normative requirements. Record unexplained server-specific
behavior as an open interoperability question.

## 9. PJSIP and dependency evaluation

Before adoption, verify PJSIP against the actual requirements:

- Required transports (UDP/TCP/TLS), IPv4/IPv6, and certificate validation.
- SIP transactions, dialogs, authentication, session timers, and required
  extensions.
- Custom headers, content types, provisional responses, and SDP integration.
- Threading/event-loop integration and clean runtime shutdown.
- Platform/compiler support for development and deployment targets.
- License and redistribution obligations for PJSIP and transitive components.
- Version pinning, reproducible builds, vulnerability updates, and packaging.

PJSIP can provide SIP infrastructure; the project must still implement and
test the MCX-specific procedures. Evaluate RTP/media support independently
rather than assuming all required MCX codecs and security profiles are covered.

Use CMake targets to keep boundaries visible:

```text
mcx_lib          public MCX library
mcx_pjsip        internal PJSIP adapter/dependency boundary, if practical
mcx_lib_mock     manual mock consumer
mcx_*_test       automated test executables
```

The exact target layout should follow the selected PJSIP packaging approach.
Do not make a PJSIP dependency leak into public headers.

## 10. Testing strategy

### Unit and contract tests

Cover configuration, API states, operation IDs, event order, error mapping,
time-outs, cancellation, and shutdown. Keep these tests independent of a
running IMS or MCX server.

### Protocol tests

Use a local scripted SIP peer and sanitized message vectors to test request
construction, parsing, retransmission/error handling, SDP, and MCX content.
Assert protocol semantics and state transitions, not incidental formatting.

### Interoperability tests

Against a controlled target environment, test the selected registration and
MCX use cases, rejected credentials, expired tokens, network/server loss, and
recovery. Preserve sanitized traces with version/profile metadata.

### Security and robustness tests

Test malformed and oversized configuration/protocol inputs, bounded
allocations, TLS verification, secret redaction, duplicate/late messages,
callback shutdown safety, and dependency vulnerability response.

## 11. Configuration and security

Evolve the current INI configuration only as requirements are confirmed.
Separate logical sections for:

```text
endpoint      identities and service identities
server        registrar, proxy, and MCX service endpoints
transport     protocol, bind addresses, and ports
authorization credential/token provider and expiry policy
registration  expiry, retry, and timeout policy
media         codecs, RTP range, and media settings, if in scope
security      trust roots and certificate/key references
logging       level and secret-safe diagnostics
```

Do not store production secrets in source-controlled configuration. Do not
print passwords, access/refresh tokens, client secrets, private keys, or
authorization headers. Prefer an injected credential provider or protected
secret store; redact centrally.

## 12. Immediate next steps

1. Update the README to describe the library as a component consumed by an
   external gateway.
2. Write the MVP/profile decisions from Section 4, especially the first
   registration or call use case and the target test environment.
3. Define a small, stable public client lifecycle API and test it with a
   deterministic mock.
4. Evaluate and pin PJSIP for the actual target platforms and license terms.
5. Build a standalone SIP probe against a local scripted peer.
6. Implement IMS registration using the agreed authentication/token flow.
7. Implement one MCX server use case end to end and expand only after it is
   interoperable and tested.

The first meaningful milestone is:

```text
The library builds independently
        +
An example consumer can start/stop it and receive deterministic events
        +
A PJSIP-backed probe completes a controlled SIP flow
        +
The library registers against the agreed test environment
        +
One agreed MCX procedure succeeds and fails predictably under test
```
