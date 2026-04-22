# BDD TLS test material

**TEST ONLY.** These keys and certificates exist solely to drive the BDD TLS
and mTLS scenarios (`Bdd/features/tls_transport.feature`,
`Bdd/features/mtls_transport.feature`) against syslog-ng running in a BDD
compose container. They must never be used for any purpose that touches real
data.

| File | Role |
|---|---|
| `ca.key` | Test CA private key. Signs the server and client certs. |
| `ca.pem` | Test CA certificate. Trust anchor for both the example binary (verifies the server) and syslog-ng (verifies the client in mTLS mode). |
| `server.key` | syslog-ng server private key. |
| `server.pem` | syslog-ng server certificate, signed by the test CA. SANs: `syslog-ng`, `localhost`, `127.0.0.1`. |
| `client.key` | Client (SolidSyslog-under-test) private key for mTLS. |
| `client.pem` | Client certificate for mTLS, signed by the test CA. CN: `solidsyslog-bdd-client`. |

Validity is 10 years from generation, so these effectively never expire for
test purposes.

## Regenerating

Run `./generate.sh` from this directory (or the repo root). Commit the
regenerated files — the BDD suite pins them.
