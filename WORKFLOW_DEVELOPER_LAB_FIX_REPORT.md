# LeoMiniGames v0.7.1 — Workflow & Developer Lab Fix Report

Date: 2026-09-22
Baseline inspected: `d22534e2170948f2ab6c0f8b333cec5380e88f6a`

## GitHub Actions diagnosis

### CI #24 — run 35752864666

Only the Android x86_64 lane failed. Configuration and native compilation reached Android packaging, then `androiddeployqt`/Gradle failed while downloading Gradle 8.14.3 because the upstream distribution URL returned HTTP 500. The remaining inspected platform lanes succeeded, including the other Android ABIs and universal multi-ABI build.

Resolution: Android CI and Android release packaging now route Gradle-backed APK/AAB targets through `tools/ci/android_build_with_retry.sh`. Retry is bounded and signature-based: only recognized Gradle-distribution/network failures are retried. Compile/link/package errors that do not match the transient network signatures fail immediately.

Controlled regression tests:

- HTTP 500 on attempts 1 and 2, success on attempt 3 → PASS, exactly 3 attempts.
- simulated C++ compile failure → PASS, exit on attempt 1 with no retry.

### Publish Release #4 — run 35755442043

Preflight stopped with `No successful exact-SHA run for .github/workflows/ci.yml` because CI #24 was red. That gate is intentional and remains intact. The fix addresses the underlying transient Android CI failure rather than weakening release safety.

## Developer Lab diagnosis

The native Developer Lab used `GET /api/v1/developer/contents` to decide whether an API key was valid. That endpoint is a publishing/content endpoint: API-key access goes through `apiKeyPrincipal("content:read")`, which requires the `content:read` scope, active developer onboarding and publisher authorization. It is therefore not a valid authentication probe.

A second deployment-specific problem existed in backend bearer extraction: it only read `HTTP_AUTHORIZATION`. Apache/FastCGI/cPanel can forward the Authorization header through `REDIRECT_HTTP_AUTHORIZATION` or the request header map, causing a valid key to appear absent.

## Developer Lab resolution

The client now verifies against:

`GET /api/v1/developer/auth/verify`

The backend endpoint validates the active API key without requiring a publishing/content scope, checks the enabled developer account and active publisher membership, and returns an explicit no-store authentication contract containing developer, publisher and key metadata. The raw credential is never returned or persisted by the client.

Bearer extraction accepts the expected Apache/FastCGI forwarding variants while retaining strict `Bearer <token>` parsing. `public/.htaccess` also preserves the Authorization header for the PHP application.

The existing Developer Lab RCC boundaries remain unchanged: session-only local RCC, hashed session copy, collision checks, no Native/L3 elevation, and no raw API-key persistence.

## Validation

Client:

- source/project/sanity: PASS
- security: 16 PASS / 0 ERROR
- distribution: 245 PASS / 0 ERROR
- v0.7.1 validator: PASS
- workflow YAML: PASS

Backend:

- PHP syntax sweep: PASS
- Developer Lab auth-header tests: PASS
- Developer Lab static contract validator: PASS
- Update Gateway tests/validator: PASS

## Runtime honesty

This report is based on the real GitHub Actions logs for the failing baseline plus source/static/unit-style validation of the fixes. The updated tree has not yet produced a post-fix GitHub-hosted run because it must first be committed and pushed. No physical-device Developer Lab or gameplay test is claimed.
