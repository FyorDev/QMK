default:
    @just --list

# Check required tools, then enable the githooks
setup:
    #!/usr/bin/env bash
    set -euo pipefail
    missing=()
    for tool in git rumdl shfmt shellcheck nixfmt statix deadnix gh fgj; do
        command -v "$tool" >/dev/null || missing+=("$tool")
    done
    [ ${#missing[@]} -eq 0 ] || { echo "missing tools: ${missing[*]}"; exit 1; }
    git config core.hooksPath .githooks
    echo "enabled .githooks"

# Format markdown, shell, nix, just files
fmt:
    git ls-files -z '*.md' | xargs -0 -r rumdl fmt --disable MD013,MD028
    git ls-files -z '*.sh' '.githooks/*' | xargs -0 -r shfmt -w
    git ls-files -z '*.nix' | xargs -0 -r nixfmt
    just --fmt

# Lint markdown, shell, nix files
lint:
    git ls-files -z '*.md' | xargs -0 -r rumdl check --disable MD013,MD028
    git ls-files -z '*.sh' '.githooks/*' | xargs -0 -r shellcheck
    git ls-files -z '*.nix' | xargs -0 -r -n1 statix check
    git ls-files -z '*.nix' | xargs -0 -r deadnix --fail

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
