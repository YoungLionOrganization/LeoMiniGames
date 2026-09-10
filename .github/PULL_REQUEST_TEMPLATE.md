## Summary

Describe what changed and why.

## Compatibility

- [ ] Existing v0.5/v0.6 RCC content is not intentionally broken.
- [ ] Manifest/RCC/theme/i18n/save/audio/input compatibility was considered.
- [ ] Any unavoidable breaking change has a migration/fallback path.

## Validation

- [ ] `python3 tools/source_guard.py`
- [ ] `python3 tools/validate_project.py`
- [ ] `python3 tools/security_audit.py`
- [ ] `python3 tools/validate_v070.py`
- [ ] CMake configure/build completed on at least one relevant platform.
- [ ] Runtime/device testing is stated accurately; untested platforms are not marked PASS.

## Security / trust

- [ ] Publisher verification is not accepted from client-controlled manifests.
- [ ] Native/L3 capability changes are explicit and reviewed.
- [ ] No secrets, keystores, tokens or generated credentials are committed.
