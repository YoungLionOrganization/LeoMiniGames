# Publisher Terms Acceptance & Evidence

Use versioned, server-side evidence.

Recommended record:

```json
{
  "acceptance_id": "uuid",
  "younglion_user_id": "canonical Supabase auth.users.id",
  "publisher_id": "uuid",
  "document_id": "younglion-developer-publisher-terms",
  "document_version": "1.0",
  "document_sha256": "<64 hex>",
  "accepted_at": "<UTC timestamp>",
  "acceptance_method": "developer-portal-explicit",
  "scope": "publisher",
  "canonical_language": "en"
}
```

Do not store raw API keys in acceptance records.

CLI/API publishing must check server-side acceptance for the credential owner.
An untrusted request body field such as `"accepted": true` is not sufficient.

Native/L3 requires both Publisher Terms and Native/L3 Addendum acceptance plus a
separate backend capability grant.

Historical acceptance records should be append-only/auditable rather than
silently rewritten.
