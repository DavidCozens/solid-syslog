#!/usr/bin/env bash
#
# Regenerates the test-only self-signed CA + syslog-ng server cert used by the
# TLS BDD scenarios. Keys in this directory are FOR TESTING ONLY — never use
# them for anything that touches real data.
#
# Run from the repo root:   ./Bdd/syslog-ng/tls/generate.sh
# Or from this directory:   ./generate.sh
#
# Validity is 10 years so the certs effectively never expire for test purposes.
# If you regenerate, commit the changes — the BDD suite pins these files.

set -euo pipefail

cd "$(dirname "$0")"

DAYS=3650
SUBJECT_CA="/CN=SolidSyslog BDD Test CA"
SUBJECT_SRV="/CN=syslog-ng"
SUBJECT_CLIENT="/CN=solidsyslog-bdd-client"
SAN="subjectAltName = DNS:syslog-ng, DNS:localhost, IP:127.0.0.1"

# 1. CA key + self-signed CA cert
openssl genrsa -out ca.key 2048
openssl req -x509 -new -nodes -key ca.key -sha256 -days "$DAYS" \
    -subj "$SUBJECT_CA" -out ca.pem

# 2. Server key + CSR
openssl genrsa -out server.key 2048
openssl req -new -key server.key -subj "$SUBJECT_SRV" -out server.csr

# 3. Sign server cert with the CA, carrying the SAN
openssl x509 -req -in server.csr -CA ca.pem -CAkey ca.key -CAcreateserial \
    -out server.pem -days "$DAYS" -sha256 \
    -extfile <(printf "%s\n" "$SAN")

# 4. Client key + CSR (mTLS identity for SolidSyslog under test)
openssl genrsa -out client.key 2048
openssl req -new -key client.key -subj "$SUBJECT_CLIENT" -out client.csr

# 5. Sign client cert with the same CA — no SAN needed for client auth
openssl x509 -req -in client.csr -CA ca.pem -CAkey ca.key -CAcreateserial \
    -out client.pem -days "$DAYS" -sha256

# Remove the CSRs and CA serial — only the keys + certs need to be committed
rm -f server.csr client.csr ca.srl

chmod 0644 ca.pem server.pem client.pem
chmod 0600 ca.key server.key client.key

echo "Regenerated test certs in $(pwd)"
