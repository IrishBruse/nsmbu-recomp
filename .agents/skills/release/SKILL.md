---
name: release
description: >-
  Cut an NSMBU GitHub release: move CHANGELOG Unreleased to a version, commit,
  push tag v*, and watch the release workflow. Use when the user asks to
  release, cut a version, push a release tag, watch release CI, or rerun a
  release until it is published and green.
---

# Release

Ship a versioned GitHub Release from `main` by pushing a `v*` tag.
The workflow is `.github/workflows/release.yml`.
Local pushes run `tools/git/pre-push`, which builds and runs the full `ctest` suite (path-alias tests such as `mod_content_startup` first).

## Steps

1. Confirm the version string with the user if they did not name one (example `0.4.0`).
   The git tag is `v` plus that string (`v0.4.0`).
   The CHANGELOG heading is the string without `v` (`## 0.4.0`).
2. On a clean intent to release from current work:
   - Move every note under `## Unreleased` in `CHANGELOG.md` under a new `## X.Y.Z` heading.
   - Leave `## Unreleased` empty above it.
   - Commit that change with any other release-ready files the user asked to include.
3. For a player-facing write-up of that version, follow `.agents/skills/changelog-summary/SKILL.md`.
4. Push `main` to `origin` when the release commit is not yet on the remote.
5. Create the tag on that commit: `git tag vX.Y.Z`.
   Push only that tag: `git push origin vX.Y.Z`.
6. Find the new `release` workflow run for the tag (`gh run list --workflow=release.yml --limit 3`).
   Watch it: `gh run watch <id> --exit-status`.
7. On success: report the run URL and the GitHub Release URL (`gh release view vX.Y.Z --json url -q .url`).
   Confirm the release is not a draft (`gh release view vX.Y.Z --json isDraft -q .isDraft` is `false`).
8. On failure:
   - Report the run URL, which jobs failed, and the failing test or step from `gh run view <id> --log-failed`.
   - If the user asked only to watch or cut a release: stop. Do not retag.
   - If the user asked to push and rerun until published and green (or an explicit retag): fix the failure, commit, push `main`, retag (`git tag -d vX.Y.Z`, `git push origin :refs/tags/vX.Y.Z`, `git tag vX.Y.Z`, `git push origin vX.Y.Z`), and return to step 6.
   - Repeat until the run succeeds and the release is published.

Done when the tag is on `origin`, the release workflow conclusion is `success`, and the GitHub Release exists and is not a draft.

## Rules

- Tag pattern must be `v*` or the workflow does not publish.
- `tools/release/notes.py` copies the matching `## X.Y.Z` block from `CHANGELOG.md` into the GitHub Release body. That heading must exist before the tag build publishes.
- Do not create an empty version heading.
- Do not push a tag that already exists on `origin` without an explicit retag request (or an until-green / rerun request that implies retag).
- Dry run without publishing: `gh workflow run release.yml -f dry_run=true` (no tag required). Use that only when the user asks for a dry run.

## Jobs to expect

| Job | Role |
| --- | --- |
| version | Sets `VERSION` from the tag |
| macos / windows / linux-x86_64 / linux-aarch64 | Build, package, unit tests |
| virustotal | After package jobs |
| publish | Upload zips / AppImage and `gh release create` |
| vt-notes | Append VirusTotal section to the release notes |

Any failed package job skips publish.
