default:
    @just --list

# Check required tools, then enable the githooks
setup:
    #!/usr/bin/env bash
    set -euo pipefail
    missing=()
    for tool in git rumdl shfmt shellcheck nixfmt statix deadnix clang-format cppcheck gh fgj; do
        command -v "$tool" >/dev/null || missing+=("$tool")
    done
    [ ${#missing[@]} -eq 0 ] || { echo "missing tools: ${missing[*]}"; exit 1; }
    git config core.hooksPath .githooks
    echo "enabled .githooks"

# Format markdown, shell, nix, C, just files
fmt:
    git ls-files -z '*.md' | xargs -0 -r rumdl fmt --disable MD013,MD028
    git ls-files -z '*.sh' '.githooks/*' | xargs -0 -r shfmt -w
    git ls-files -z '*.nix' | xargs -0 -r nixfmt
    git ls-files -z '*.c' '*.h' | xargs -0 -r clang-format -i
    just --fmt

# Lint markdown, shell, nix, C files
lint:
    git ls-files -z '*.md' | xargs -0 -r rumdl check --disable MD013,MD028
    git ls-files -z '*.sh' '.githooks/*' | xargs -0 -r shellcheck
    git ls-files -z '*.nix' | xargs -0 -r -n1 statix check
    git ls-files -z '*.nix' | xargs -0 -r deadnix --fail
    git ls-files -z '*.c' '*.h' | xargs -0 -r clang-format --dry-run --Werror
    git ls-files -z '*.c' '*.h' | xargs -0 -r cppcheck --error-exitcode=1 --quiet --suppressions-list=.cppcheck-suppressions -DPROGMEM= --enable=warning,performance,portability

# Build the K5 Max firmware
build-k5:
    nix build .#k5-max -o result-k5

# Build the Corne firmware
build-corne:
    nix build .#corne -o result-corne

# Flash the K5 Max over DFU
flash-k5: build-k5
    @echo "Keychron K5 should be plugged in while holding ESC key"
    nix shell nixpkgs#dfu-util -c dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D result-k5/k5-max.bin

# Mount the Corne bootloader first
mount-corne:
    @echo "Put the corne in bootloader mode (double tap button next to liatris)..."
    @until [ -e /dev/disk/by-label/RPI-RP2 ]; do sleep 1; done
    @findmnt -rn /run/media/$USER/RPI-RP2 >/dev/null || udisksctl mount -b /dev/disk/by-label/RPI-RP2

# Flash the Corne by copying the UF2 to its bootloader drive
flash-corne: build-corne mount-corne
    cp result-corne/corne.uf2 /run/media/$USER/RPI-RP2/

# Register Corne to OpenRGB
openrgb-corne:
    sudo python3 scripts/openrgb-corne.py

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
