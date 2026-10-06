# Project

## Install

```sh
just setup
```

Requires:

```text
just
gh fgj
git-lfs
rumdl
shfmt shellcheck
```

## Release

```sh
just release --dry v0.0.0 "title"
just release v0.0.0 "title"
```

Commits follow [Conventional Commits](https://www.conventionalcommits.org/)

- Example: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`, `ci`, `build`, `revert`
