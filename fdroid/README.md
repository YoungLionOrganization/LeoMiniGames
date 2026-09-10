# F-Droid packaging

See [`../F_DROID_READINESS.md`](../F_DROID_READINESS.md) for the authoritative status.

The metadata scaffold already contains the real YoungLionOrganization repository URLs and v0.7.0/700 identifiers. Only `FULL_COMMIT_SHA` is intentionally unresolved until the final submission commit exists. The final fdroiddata recipe must build Qt from source (the current fdroiddata precedent is the `Qt5@v6.10.1` srclib used for Qt 6 projects), then call `tools/fdroid_clean_build.sh`. Do not commit/download an opaque Qt Online Installer SDK to make the recipe pass.
