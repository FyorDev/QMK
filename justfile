default:
    @just --list

# Check required tools and LFS, then enable the githooks
setup:
    #!/usr/bin/env bash
    set -euo pipefail
    missing=()
    for tool in git git-lfs git-cliff rumdl shfmt shellcheck gh fgj; do
        command -v "$tool" >/dev/null || missing+=("$tool")
    done
    [ ${#missing[@]} -eq 0 ] || { echo "missing tools: ${missing[*]}"; exit 1; }
    git config --get filter.lfs.clean >/dev/null && [ "$(git config --get filter.lfs.required)" = true ] || { echo "git lfs is not set up, run: git lfs install"; exit 1; }
    git config core.hooksPath .githooks
    echo "enabled .githooks"

# Format markdown, shell, just files
fmt:
    git ls-files -z '*.md' | xargs -0 -r rumdl fmt --disable MD013,MD028
    git ls-files -z '*.sh' '.githooks/*' | xargs -0 -r shfmt -w
    just --fmt

# Lint markdown, shell files
lint:
    git ls-files -z '*.md' | xargs -0 -r rumdl check --disable MD013,MD028
    git ls-files -z '*.sh' '.githooks/*' | xargs -0 -r shellcheck

# Preview changelog since last version
changelog:
    git cliff --unreleased -o -

# Make a release `just release v1.2.3 ["Title"]`; preview with `just release --dry ...`
[arg("dry", long="dry", value="true")]
release version title="" dry="false":
    #!/usr/bin/env bash
    set -euo pipefail
    dry={{ dry }}
    version={{ quote(version) }}
    title={{ quote(title) }}
    label="$version${title:+: $title}"

    [[ $version =~ ^v[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.-]+)?$ ]] || { echo "version must look like v1.2.3"; exit 1; }
    [ "$(git branch --show-current)" = main ] || { echo "not on main"; exit 1; }
    [ -z "$(git status --porcelain)" ] || { echo "working tree is not clean"; exit 1; }
    ! git rev-parse -q --verify "refs/tags/$version" >/dev/null || { echo "tag $version already exists"; exit 1; }

    remote=$(git remote get-url origin 2>/dev/null) || { echo "no origin remote"; exit 1; }
    host=$(printf '%s' "$remote" | sed -E 's#^[a-z+]+://##; s#^[^@/]*@##; s#[:/].*##')
    if [ "$host" = github.com ]; then forge=gh; else forge=fgj; fi
    command -v "$forge" >/dev/null || { echo "$forge is not installed (needed for $host)"; exit 1; }
    auth=$("$forge" auth status 2>&1) && [[ $auth != *"Not authenticated"* ]] || { echo "$forge is not authenticated, run: $forge auth login"; exit 1; }

    if [ "$dry" = true ]; then
        echo "dry run, nothing will be changed, changelog preview:"
        git cliff --unreleased --tag "$version" -o -
        echo "dry run, would run (release via $forge for $host):"
        echo "git add CHANGELOG.md"
        echo "git commit -m \"chore(release): prepare for $version\""
        echo "git tag -s $version -m \"$label\""
        echo "git push origin main"
        echo "git push origin $version"
        echo "$forge release create $version -t \"$label\" -F ./CHANGELOG.md"
    else
        git cliff --tag "$version" -o CHANGELOG.md
        git add CHANGELOG.md
        git commit -m "chore(release): prepare for $version"
        git tag -s "$version" -m "$label"
        git push origin main
        git push origin "$version"
        "$forge" release create "$version" -t "$label" -F ./CHANGELOG.md
    fi

# Open the repo in your browser, using either gh or fgj
browse:
    #!/usr/bin/env bash
    set -euo pipefail
    remote=$(git remote get-url origin 2>/dev/null) || { echo "no origin remote"; exit 1; }
    host=$(printf '%s' "$remote" | sed -E 's#^[a-z+]+://##; s#^[^@/]*@##; s#[:/].*##')
    if [ "$host" = github.com ]; then
        gh browse
    else
        url=$(fgj repo view --json | sed -n 's/^  "html_url": "\(.*\)",\{0,1\}$/\1/p')
        [ -n "$url" ] || { echo "could not read the repo url from fgj"; exit 1; }
        opener=$(command -v xdg-open || command -v open) || { echo "no browser opener found, url: $url"; exit 1; }
        "$opener" "$url"
    fi
