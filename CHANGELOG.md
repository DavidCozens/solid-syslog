# Changelog

## 1.0.0 (2026-04-12)


### Features

* add clone initialisation script and restructure documentation ([#9](https://github.com/DavidCozens/solid-syslog/issues/9)) ([7731bd8](https://github.com/DavidCozens/solid-syslog/commit/7731bd8a5461b4b56ca75a5d1ff185c3798c502f))
* generalise SD config from single pointer to array (S7.2) ([#73](https://github.com/DavidCozens/solid-syslog/issues/73)) ([4808be8](https://github.com/DavidCozens/solid-syslog/commit/4808be858fd05a26f0b3b1766779371a064dbfc1))
* initialise SolidSyslog component ([57dd7c9](https://github.com/DavidCozens/solid-syslog/commit/57dd7c9666097763e5f4547939564457a93bc5ff))
* origin structured data — software and swVersion (S7.3) ([#76](https://github.com/DavidCozens/solid-syslog/issues/76)) ([7f7834b](https://github.com/DavidCozens/solid-syslog/commit/7f7834b61750e8187e530346d78b0884829e875e))
* S1.1 walking skeleton — RFC 5424 message validated field by field ([#30](https://github.com/DavidCozens/solid-syslog/issues/30)) ([3f0b318](https://github.com/DavidCozens/solid-syslog/commit/3f0b3182aac011457f22930c679f76d6ca11188d))
* S1.2 PRIVAL encoding — facility and severity on the Log call ([#42](https://github.com/DavidCozens/solid-syslog/issues/42)) ([4fa7247](https://github.com/DavidCozens/solid-syslog/commit/4fa7247c2c7b4945a491a1d0f4dac38b28edcc64))
* S1.3 Timestamp — raise-time capture via injected clock ([#44](https://github.com/DavidCozens/solid-syslog/issues/44)) ([d776503](https://github.com/DavidCozens/solid-syslog/commit/d7765031aadf036cd8317393db5c7d91fd4dae8b))
* S1.4 Hostname, AppName, ProcId — injected via config function pointers ([#45](https://github.com/DavidCozens/solid-syslog/issues/45)) ([1f79a23](https://github.com/DavidCozens/solid-syslog/commit/1f79a23a70339302ed7aa82324d590e41b33b7a6))
* S1.5 MessageId and Message — fields on SolidSyslogMessage ([#47](https://github.com/DavidCozens/solid-syslog/issues/47)) ([0d556ef](https://github.com/DavidCozens/solid-syslog/commit/0d556ef0b03d90589780e046767e52eaee2d85e3))
* S15.1 TCP sender with RFC 6587 octet-counting framing ([#91](https://github.com/DavidCozens/solid-syslog/issues/91)) ([1d77d3d](https://github.com/DavidCozens/solid-syslog/commit/1d77d3d4740b0695c250f93ffac0846cf8457cc5))
* S15.2 Connection failure detection and reconnection ([#93](https://github.com/DavidCozens/solid-syslog/issues/93)) ([07768dd](https://github.com/DavidCozens/solid-syslog/commit/07768dda4880582510f94268d65ec459dd6e1175))
* S2.1 walking skeleton — SolidSyslogUdpSender transmits a buffer ([#32](https://github.com/DavidCozens/solid-syslog/issues/32)) ([ebc2137](https://github.com/DavidCozens/solid-syslog/commit/ebc213729e0faeeb167f4968efb1401602abef9d))
* S2.2 inject host and port into UdpSender via config function pointers ([#35](https://github.com/DavidCozens/solid-syslog/issues/35)) ([fd508ad](https://github.com/DavidCozens/solid-syslog/commit/fd508ad4d98e2c8ad97f916ba4157e4a4c4d53f8))
* S2.3 CMake platform detection for PosixUdpSender ([#39](https://github.com/DavidCozens/solid-syslog/issues/39)) ([7c05c48](https://github.com/DavidCozens/solid-syslog/commit/7c05c487dd1aa7fe960df02828750adbd6d5f78c))
* S2.4 BDD walking skeleton — Behave validates syslog-ng output ([#38](https://github.com/DavidCozens/solid-syslog/issues/38)) ([df1fb56](https://github.com/DavidCozens/solid-syslog/commit/df1fb56fd09f8d2c133b426fe342d395aa3171bd))
* S4.1 buffer abstraction and NullBuffer ([#57](https://github.com/DavidCozens/solid-syslog/issues/57)) ([32237ce](https://github.com/DavidCozens/solid-syslog/commit/32237cee3ef4c1e84b65f2886076bb9724c0a88d))
* S4.2 SolidSyslog_Service API and Buffer Read ([#58](https://github.com/DavidCozens/solid-syslog/issues/58)) ([5293a1d](https://github.com/DavidCozens/solid-syslog/commit/5293a1dcb5509caf0694ef7095fc93e097d9d867))
* S4.3 PosixMqBuffer — POSIX message queue buffer ([#59](https://github.com/DavidCozens/solid-syslog/issues/59)) ([35228e5](https://github.com/DavidCozens/solid-syslog/commit/35228e5ba8ce5d817aad5798570251f77d62f07e))
* S4.4 BDD — buffered message delivery through syslog-ng ([#62](https://github.com/DavidCozens/solid-syslog/issues/62)) ([7bb2254](https://github.com/DavidCozens/solid-syslog/commit/7bb22542e00839404b9d95319fbe821112d7c1bc))
* S4.4 example restructure — shared code, threaded example, test harness ([#61](https://github.com/DavidCozens/solid-syslog/issues/61)) ([ed8b559](https://github.com/DavidCozens/solid-syslog/commit/ed8b559cfc954f47f268a55609d93524afdad8d9))
* S5.2 Store abstraction, NullStore, and Service algorithm ([#84](https://github.com/DavidCozens/solid-syslog/issues/84)) ([621ada7](https://github.com/DavidCozens/solid-syslog/commit/621ada7b5d96ee068997e9573273973121df75a4))
* S5.3 File-based store with sender outage BDD ([#98](https://github.com/DavidCozens/solid-syslog/issues/98)) ([51ac3e1](https://github.com/DavidCozens/solid-syslog/commit/51ac3e1c9cdaf53785302da63f8ad021092de5e2))
* S5.4 Power cycle replay BDD scenario ([#100](https://github.com/DavidCozens/solid-syslog/issues/100)) ([072fc29](https://github.com/DavidCozens/solid-syslog/commit/072fc29aebe086122be88df73fe605bc3a814254))
* S5.5 file rotation with discard policies ([#103](https://github.com/DavidCozens/solid-syslog/issues/103)) ([ecfce10](https://github.com/DavidCozens/solid-syslog/commit/ecfce10b99b39982162d08dfca3a67db25447472))
* S5.6 file store corruption detection and recovery ([#106](https://github.com/DavidCozens/solid-syslog/issues/106)) ([5ea7e7a](https://github.com/DavidCozens/solid-syslog/commit/5ea7e7aa4ad4c6e6590c1020f676ef65b9da8c9f))
* S5.7 halt callback on storage full ([#107](https://github.com/DavidCozens/solid-syslog/issues/107)) ([fc30831](https://github.com/DavidCozens/solid-syslog/commit/fc30831e3d462d6792069cc4b334aaf1dfa928ca))
* structured data with sequenceId (S7.1) ([#72](https://github.com/DavidCozens/solid-syslog/issues/72)) ([8b96405](https://github.com/DavidCozens/solid-syslog/commit/8b96405ccf0c6dc2936642b952ab22d26dbff547)), closes [#65](https://github.com/DavidCozens/solid-syslog/issues/65)


### Bug Fixes

* correct clone init script and CI for private repos ([#13](https://github.com/DavidCozens/solid-syslog/issues/13)) ([389e043](https://github.com/DavidCozens/solid-syslog/commit/389e043a0570bc9b20c553907a1fceabdacefef3))
* S5.5 FormatFilename buffer overflow on long pathPrefix ([#104](https://github.com/DavidCozens/solid-syslog/issues/104)) ([56a38e3](https://github.com/DavidCozens/solid-syslog/commit/56a38e38e549061ac0bdc4f03b1ed76a3dc4d822))


### CI / Build

* add missing needs dependencies and resilient artifact downloads ([#71](https://github.com/DavidCozens/solid-syslog/issues/71)) ([37e8718](https://github.com/DavidCozens/solid-syslog/commit/37e87181e9fee3d779d983793509c8cf40caf2b2))
* harden BDD infrastructure robustness ([#94](https://github.com/DavidCozens/solid-syslog/issues/94)) ([51d4c96](https://github.com/DavidCozens/solid-syslog/commit/51d4c966e02fc9fb9f48c9d1510ca61abc1f4ea0))
* harden build tooling and add Clang, release-please, CLAUDE.md ([a7e3815](https://github.com/DavidCozens/solid-syslog/commit/a7e38154142f8281ce5a1a7304caa105e94b541a))
* lock down GitHub Actions token permissions to least privilege ([#8](https://github.com/DavidCozens/solid-syslog/issues/8)) ([a079ff6](https://github.com/DavidCozens/solid-syslog/commit/a079ff602911ee1c31ead9edd9ae5a8bcb3500a0))
* pin all GitHub Actions to SHA digests ([b02ad19](https://github.com/DavidCozens/solid-syslog/commit/b02ad19431b852ff14bbb4b7acddcba31e123680))
* promote BDD to required status check ([#46](https://github.com/DavidCozens/solid-syslog/issues/46)) ([f184804](https://github.com/DavidCozens/solid-syslog/commit/f184804fabbfd590121e086ba738d066dc69ed2c))
* replace EnricoMi with dorny/test-reporter for per-test visibility ([#36](https://github.com/DavidCozens/solid-syslog/issues/36)) ([8033a08](https://github.com/DavidCozens/solid-syslog/commit/8033a08e2227a5669787421720a71fba143b2510))
* show CppUTest summary in default VS Code build task ([#37](https://github.com/DavidCozens/solid-syslog/issues/37)) ([0e2ebe9](https://github.com/DavidCozens/solid-syslog/commit/0e2ebe9d6b079459a6f6c46107e9e884e0ff75fe))
* unified quality dashboard with Quality Monitor ([#63](https://github.com/DavidCozens/solid-syslog/issues/63)) ([fb8c0a1](https://github.com/DavidCozens/solid-syslog/commit/fb8c0a1d71ca036c0ebd3cee0249e2a85c889bab))


### Refactoring

* align function ordering and test style with conventions ([#49](https://github.com/DavidCozens/solid-syslog/issues/49)) ([b11ea91](https://github.com/DavidCozens/solid-syslog/commit/b11ea9199fd6b7ca4476b54b7cc4e3c9ec000d69))
* change SolidSyslogClockFunction to pointer parameter ([#74](https://github.com/DavidCozens/solid-syslog/issues/74)) ([f66bec3](https://github.com/DavidCozens/solid-syslog/commit/f66bec3620a17371723b5769ad61a61981cf2d3d))
* expand abbreviated names and correct test double taxonomy ([#85](https://github.com/DavidCozens/solid-syslog/issues/85)) ([bfc243d](https://github.com/DavidCozens/solid-syslog/commit/bfc243d883423852dcbe973d9678e13623ff0a7c))
* S5.1 Sender.Send returns bool for send success/failure ([#83](https://github.com/DavidCozens/solid-syslog/issues/83)) ([355d169](https://github.com/DavidCozens/solid-syslog/commit/355d1697b48fae0e56526f6fd30b9a7196b82c90))
* single-instance SolidSyslog with Null Object pattern (S12.1) ([#78](https://github.com/DavidCozens/solid-syslog/issues/78)) ([ee7c7b0](https://github.com/DavidCozens/solid-syslog/commit/ee7c7b0a9ecef44effad5b16873a4bed822a99b9))


### Maintenance

* add .claude to gitignore and document kanban conventions in SKILL.md ([6ef1687](https://github.com/DavidCozens/solid-syslog/commit/6ef16870f7bed3848f67f3d5fb356161f5f0737c))
* clean up code style and split SolidSyslog.h (ISP) ([#43](https://github.com/DavidCozens/solid-syslog/issues/43)) ([9b9a3df](https://github.com/DavidCozens/solid-syslog/commit/9b9a3df28b3115720bf188cc1c8670798efb02c5))
* **main:** release 1.0.0 ([#5](https://github.com/DavidCozens/solid-syslog/issues/5)) ([2694a18](https://github.com/DavidCozens/solid-syslog/commit/2694a187149552819d296fc4d025229d75f955b7))
* **main:** release 1.1.0 ([#1](https://github.com/DavidCozens/solid-syslog/issues/1)) ([cf9c364](https://github.com/DavidCozens/solid-syslog/commit/cf9c364da113abff080adb999da7d8ea43e4f421))
* merge template updates (Example, BDD skeleton, container bump) ([#41](https://github.com/DavidCozens/solid-syslog/issues/41)) ([8c43472](https://github.com/DavidCozens/solid-syslog/commit/8c4347297787b7a23961f8f174fe3487e30d49ad))
* migrate container images from DockerHub to GHCR ([#14](https://github.com/DavidCozens/solid-syslog/issues/14)) ([7cb0206](https://github.com/DavidCozens/solid-syslog/commit/7cb0206ff4b0c2998729f5e566d4f06354e338a6))
* migrate devcontainer to Docker Compose with separate Clang image ([#11](https://github.com/DavidCozens/solid-syslog/issues/11)) ([ecdf7c2](https://github.com/DavidCozens/solid-syslog/commit/ecdf7c27eb5ceaf12fd19cf854a8a079902ab7b8))
* migrate devcontainer to Docker Compose with separate Clang image ([#7](https://github.com/DavidCozens/solid-syslog/issues/7)) ([dae7a99](https://github.com/DavidCozens/solid-syslog/commit/dae7a99f7d004cf2f3aefd6190a7c7af86f0ad1d))
* remove stale devcontainer on initialize to fix post-restart SSH bind mount failure ([#28](https://github.com/DavidCozens/solid-syslog/issues/28)) ([5686c87](https://github.com/DavidCozens/solid-syslog/commit/5686c87206d167abb83aab20f2d383eb4af5538d)), closes [#27](https://github.com/DavidCozens/solid-syslog/issues/27)
* reset release versioning to 0.x ([#99](https://github.com/DavidCozens/solid-syslog/issues/99)) ([1a4f507](https://github.com/DavidCozens/solid-syslog/commit/1a4f507bd0e6eb3b567f983f468c2e571cde9094))


### Documentation

* add DEVLOG with GitHub project and epic issue setup ([bae8577](https://github.com/DavidCozens/solid-syslog/commit/bae85775a78c2132261d9fbe4319520818cb03f2))
* add DEVLOG with GitHub project and epic issue setup ([4f52eb1](https://github.com/DavidCozens/solid-syslog/commit/4f52eb19b0341cb1caf47a94ea28b7fa4f38e81c))
* add prerequisites and platform support to cloning guide ([#17](https://github.com/DavidCozens/solid-syslog/issues/17)) ([22c1246](https://github.com/DavidCozens/solid-syslog/commit/22c1246a8038127b5ff29eff4e12be1c57192f71))
* complete E0 walking skeleton — repository setup ([#14](https://github.com/DavidCozens/solid-syslog/issues/14)) ([8da7761](https://github.com/DavidCozens/solid-syslog/commit/8da77613a0cded4ac0574ca5f6bd2cca2d60f092))
* expand template-updates with full cloning instructions ([#10](https://github.com/DavidCozens/solid-syslog/issues/10)) ([41345bb](https://github.com/DavidCozens/solid-syslog/commit/41345bba24f055b13226b64e4f0e8a2a42fa3e5b))
* fix consistency issues across CLAUDE.md, builds.md and CMakeLists ([#12](https://github.com/DavidCozens/solid-syslog/issues/12)) ([d21e45a](https://github.com/DavidCozens/solid-syslog/commit/d21e45ad504c2b2b61e759c8df4f54c2b559e27f))
* improve cloning guide with Ctrl+Shift+B, TDD prompt and gh CLI options ([#16](https://github.com/DavidCozens/solid-syslog/issues/16)) ([4bbde34](https://github.com/DavidCozens/solid-syslog/commit/4bbde34f774e05af228b0c9eea73f3674f06704d))
* split template-updates into cloning and updating guides ([#15](https://github.com/DavidCozens/solid-syslog/issues/15)) ([dd5eb89](https://github.com/DavidCozens/solid-syslog/commit/dd5eb897a8b4601e4159c9a2e94f950e5c2c55d8))
* update DEVLOG for E1 story decomposition ([#21](https://github.com/DavidCozens/solid-syslog/issues/21)) ([37d5bd5](https://github.com/DavidCozens/solid-syslog/commit/37d5bd5275e08c8dff002b388ff19a93e97169aa))
* update DEVLOG for E2 story decomposition ([#26](https://github.com/DavidCozens/solid-syslog/issues/26)) ([25cec70](https://github.com/DavidCozens/solid-syslog/commit/25cec7031c5477bf77c54375136dc22ab8f5724e))

## Changelog
