## Play status

The first sentence under `## Status` in `README.md` is the current play status.
Update this sentence when play progress changes.
Include the release version in that sentence.

## Map

Read `docs/nsmbu.md` for the dump layout, the build, and the upstream base.
Read the justfile for dev commands.

## Names and addresses

Use `references/headers` ([nsmbu/headers](https://github.com/nsmbu/headers)) for guest type names, function names, and `Address:` values.
Those headers match USA v1.3.0 (`red-pro2.rpx`).
Prefer them over Wind Waker HD leftovers or guessed addresses when you add hooks, sites, or runtime guest literals.

## Code comments

Write code without comments.
Remove non-license comments from code you change.

Update the CHANGELOG.md with the current feature dont add every single change add it to unreleased section always.

## Versions

Do not change version numbers unless the user approves it.
This includes the mod manager version, package `minimum_manager_version`, release tags, and similar version fields.