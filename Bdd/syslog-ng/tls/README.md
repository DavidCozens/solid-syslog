# BDD TLS test material

**TEST ONLY.** These keys and certificates exist solely to drive the BDD TLS
scenarios (`Bdd/features/tls_transport.feature` et al.) against syslog-ng
running in a BDD compose container. They must never be used for any purpose
that touches real data.

| File | Role |
|---|---|
| `ca.key` | Test CA private key. Signs the server cert. |
| `ca.pem` | Test CA certificate. Bundled into the example binary via `--tls-ca`. |
| `server.key` | syslog-ng server private key. |
| `server.pem` | syslog-ng server certificate, signed by the test CA. SANs: `syslog-ng`, `localhost`, `127.0.0.1`. |

Validity is 10 years from generation, so these effectively never expire for
test purposes.

## Regenerating

Run `./generate.sh` from this directory (or the repo root). Commit the
regenerated files — the BDD suite pins them.
