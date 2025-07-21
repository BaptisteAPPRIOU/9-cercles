# 🚀 Release & Versioning Guidelines for 9 Rings Project

This project uses **GitHub Actions** to manage automatic tagging and releases for each **Ring**, based on commit messages and pull requests merged into the `main` branch.

---

## 🔁 What Happens on Merge to `main`

When a pull request (PR) is merged into the `main` branch:

- ✅ GitHub Actions scans **all commit messages** in that PR
- ✅ The **last commit** must indicate which Ring (e.g., `ring1:`, `ring2:`)
- ✅ The workflow calculates the **highest semantic version bump** across all commits:
  - `BREAKING:` → **major**
  - `feat:` → **minor**
  - everything else → **patch**
- ✅ A **new version tag** is created for that Ring:
  - Example: `ring2-v1.4.0`
- ✅ A **GitHub Release** is published automatically

> 📌 **You do not need to create the tag or release manually.** Just push your commits and merge the PR — the workflow handles the rest.

---

## 🧠 Versioning Model

Each Ring has its **own version history**:

| Ring Prefix | Version Format      | Example Tag      |
|-------------|---------------------|------------------|
| `ring1:`    | `ring1-vMAJOR.MINOR.PATCH` | `ring1-v1.2.3` |
| `ring2:`    | `ring2-vMAJOR.MINOR.PATCH` | `ring2-v0.5.1` |

> You **must not mix rings in a single pull request**. One PR = One Ring.

---

## ✅ How to Commit (Before PR)

Each commit must:

1. **Start with a Ring prefix**:
   ```
   ring1: ...
   ring2: ...
   ring3: ...
   ```

2. Include a semantic versioning keyword to determine the release level:
   | Keyword       | Triggers        | Example                                      |
   |---------------|-----------------|----------------------------------------------|
   | `BREAKING:`   | Major version   | `ring1: BREAKING: refactor entire socket API` |
   | `feat:`       | Minor version   | `ring2: feat: support message broadcast`      |
   | anything else | Patch version   | `ring3: fix: correct port binding bug`        |

---

## 🚀 How to Release a Ring

### 1. Create your commits
```bash
git commit -m "ring2: feat: add reconnection strategy"
git commit -m "ring2: fix: correct retry delay"
```

### 2. Push to a feature branch
```bash
git push origin ring2/reconnect-strategy
```

### 3. Open a Pull Request into `main`

- Set target branch to `main`
- Ensure your **last commit** clearly starts with the correct `ringX:` prefix
- The workflow will use this to choose the Ring for release

### 4. Merge the PR

Once merged, the GitHub Action will:

- Detect the ring and highest bump level from your commits
- Create a new version tag (e.g. `ring2-v0.6.0`)
- Publish a GitHub Release automatically

---

## 🛑 What Not to Do

- ❌ **Do not** mix commits for multiple rings in one PR
- ❌ **Do not** manually create Git tags or releases
- ❌ **Do not** push directly to `main` (always use PRs)
- ❌ **Do not** forget the `ringX:` prefix — the workflow depends on it

---

## 📌 Summary

| Task                | Rule                                                              |
|---------------------|-------------------------------------------------------------------|
| Ring identification | Last commit in PR must start with `ringX:`                       |
| Version bump logic  | `BREAKING:` = major, `feat:` = minor, else = patch               |
| Trigger             | Merging a PR into `main`                                          |
| Tag format          | `ringX-vMAJOR.MINOR.PATCH` (e.g. `ring2-v1.0.0`)                  |
| Release             | Automatically created by GitHub Actions                          |

---
