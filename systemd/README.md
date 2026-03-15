# MapleBBS Systemd Management Guide

This guide explains how to manage the MapleBBS service suite using the modular Systemd architecture.

## Architecture Overview

The BBS suite is organized into a primary **Target** and multiple **Services**.

### Core Services (Mandatory)
These services are defined in `maplebbs.target` and will always start together:
- `maplebbs-init`: IPC/Shared memory initialization.
- `maplebbs-bbsd`: Main Telnet/SSH daemon.
- `maplebbs-gemd`: Gopher daemon.
- `maplebbs-bguard`: Guardian daemon for process monitoring.
- `maplebbs-xchatd`: Chat room daemon.
- `maplebbs-camera`: Board camera utility.
- `maplebbs-account`: Account maintenance utility.

### Optional Modules (Togglable)
These services can be enabled or disabled per instance:
- `maplebbs-bmtad`: SMTP/Mail Transfer.
- `maplebbs-bpop3d`: POP3 Mail access.
- `maplebbs-bnntpd`: NNTP News group.
- `maplebbs-innbbsd`: INN inter-BBS exchange.
- `maplebbs-ws`: WebSocket bridge (for web clients).

---

## Basic Management

### Start/Stop the whole BBS
```bash
sudo systemctl start maplebbs.target
sudo systemctl stop maplebbs.target
sudo systemctl restart maplebbs.target
```

### Check Status
```bash
# View summary of all BBS related services
systemctl list-units "maplebbs*"

# View detailed status of a specific service
systemctl status maplebbs-bbsd
```

---

## Toggling Optional Modules

To decide which optional modules run with your BBS, use the `enable/disable` commands.

### To Enable a module (e.g., SMTP)
```bash
sudo systemctl enable maplebbs-bmtad
sudo systemctl restart maplebbs.target
```

### To Disable a module (e.g., POP3)
```bash
sudo systemctl disable maplebbs-bpop3d
sudo systemctl restart maplebbs.target
```

---

## Troubleshooting

- **Logs**: Use `journalctl -u maplebbs-bbsd` to view service logs.
- **IPC Issues**: If shared memory is corrupted, `sudo systemctl restart maplebbs-init` will perform a clean sweep.

---

## Deployment Sync

If you modify service files in the source tree:
1. Sync files to `/etc/systemd/system/`.
2. Run `sudo systemctl daemon-reload`.
