# TaiWei 3D Flow Quick Start

This is a **hands-on Quick Start** for the TaiWei meta-repository. It covers:

- What each **git submodule** is for
- How to correctly **clone / init submodules**
- How to **build ORFS-Research (ORD toolchain)**
- How to **run the end-to-end 3D physical design flow** via TaiWei-Pin-3D

> Repo: https://github.com/CODA-Team/TaiWei

---

## 0. Prerequisites

### Required (common)
- Linux host (or Linux container on HPC)
- `git` (>= 2.20 recommended)
- Python 3 (and `pip`) for running experiments

### Optional
- Docker / Apptainer (recommended if you want a consistent environment)
- Cadence toolchain (only if you run the commercial `cds` flow)

---

## 1. Repository layout (submodules and their roles)

TaiWei is a **meta-repo** that pins multiple components via **git submodules** for reproducibility.

### 1.1 `ORFS-Research` — Core tools + flow infrastructure
- OpenROAD-derived research platform
- Provides the **open-source physical design engines** and flow scripts
- Used as the underlying tool stack for the **ORD** flow

### 1.2 `TaiWei-Pin-3D` — End-to-end 3D evaluation flow
- End-to-end reproducible **face-to-face (F2F) 3DIC** physical design flow
- Integrates Pin3D methodology and 2D toolchains (ORFS + optional Cadence)
- **This is the submodule you run** to execute the 3D flow

### 1.3 `TaiWei-flow-Agent` — Optional autotuning agent
- An LLM-based optimization agent for ORFS-style flows
- Optional for quick start; useful for later **autotuning / auto-exploration**

---

## 2. Setup options

Choose one setup path:

### Option A: Container-based (recommended)
Use a prebuilt container image (lab-provided or your own) so dependencies are consistent.

### Option B: Native Linux install
Install toolchain dependencies on the host directly. This is more work but convenient for developers.

This document works for both, as long as:
- `ORFS-Research` builds successfully, and
- you can run Python 3 for `TaiWei-Pin-3D`.

---

## 3. Clone TaiWei with submodules

```bash
git clone --recurse-submodules https://github.com/CODA-Team/TaiWei.git
cd TaiWei
```

---

## 4. Build ORFS-Research (ORD prerequisite)

`TaiWei-Pin-3D` requires a working **ORFS-Research** install for the open-source `ord` flow.

### 4.1 Build ORFS-Research locally

```bash
cd ORFS-Research
./setup.sh
./build_openroad.sh --local
```

### 4.2 Reference: ORFS / OpenROAD-flow-scripts installation resources

For additional installation methods and dependency guidance, refer to the official **OpenROAD-flow-scripts (ORFS)** documentation:

- **OpenROAD-flow-scripts repository (includes install instructions):**  
    https://github.com/The-OpenROAD-Project/OpenROAD-flow-scripts

## 5. Configure TaiWei-Pin-3D environment

Enter the 3D flow submodule:

```bash
cd ../TaiWei-Pin-3D
```

Edit `env.sh` and set:

* `WORK_DIR` : where outputs/logs will be placed
* `ORFS_DIR` : path to your `ORFS-Research` install directory
* `FLOW_HOME`: root of `TaiWei-Pin-3D`

```bash
source env.sh
```

---

## 6. Quick Start: run the 3D flow

All commands below assume:

* you are in `TaiWei-Pin-3D/`

### 6.1 Open-source flow (ORD): ASAP7 + ASAP7 3D stack, case = GCD

```bash
python3 run_experiments.py --flow ord --tech asap7_3D --case gcd
```

### 6.2 Commercial flow (Cadence): ASAP7 + NanGate45, case = GCD

> Requires Cadence toolchain configured in your environment.

```bash
python3 run_experiments.py --flow cds --tech asap7_nangate45_3D --case gcd
```

### 6.3 One-shot wrapper script example

```bash
bash experiment_scripts/gcd.sh
```

---

## 7. Results and visualization

After a successful run, artifacts (logs / reports / DEF/DB outputs) will be under your configured `WORK_DIR`.

Exact file names and directories depend on the selected flow and testcase.

---

## 8. Minimal checklist (I just want to run 3D flow)

1. Clone TaiWei with submodules
   `git clone --recurse-submodules ...`
2. Build ORFS-Research
   `cd ORFS-Research && ./setup.sh && ./build_openroad.sh --local`
3. Configure environment
   `cd TaiWei-Pin-3D && source env.sh`
4. Run a 3D experiment
   `python3 run_experiments.py --flow ord --tech asap7_3D --case gcd`

---

## Notes for maintainers

* Prefer **explicit commands** and **single-entry quick start** examples here so new users can run without chasing multiple repos.
* If submodule commits are updated, ensure the pinned commit versions remain consistent and reproducible across the full flow.


