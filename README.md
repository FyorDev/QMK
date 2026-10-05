# Project

Install

```sh
git config core.hooksPath .githooks
git lfs install
```

```text
git-lfs
rumdl
shfmt shellcheck
```

Release

```sh
git cliff --tag v0.0.0
git add CHANGELOG.md
git commit -m "chore(release): prepare for v0.0.0"
git tag -s v0.0.0 -m "v0.0.0"
git push origin main
git push origin v0.0.0
gh release create v0.0.0 -t "v0.0.0: <title>" -F ./CHANGELOG.md
```

Commits follow [Conventional Commits](https://www.conventionalcommits.org/)

- Example: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `chore`, `ci`, `build`, `revert`
