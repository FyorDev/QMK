{
  description = "Aurora Corne rev1 and Keychron K5 Max rgb ansi";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

    qmk = {
      url = "git+https://github.com/qmk/qmk_firmware?ref=refs/tags/0.34.6&submodules=1";
      flake = false;
    };

    keychron-qmk = {
      url = "git+https://github.com/Keychron/qmk_firmware?ref=wireless_playground&submodules=1";
      flake = false;
    };
  };

  outputs =
    {
      nixpkgs,
      qmk,
      keychron-qmk,
      ...
    }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      qmkPython = pkgs.python3.withPackages (
        ps: with ps; [
          appdirs
          argcomplete
          colorama
          dotty-dict
          hid
          hjson
          jsonschema
          milc
          pillow
          pygments
          pyserial
          pyusb
        ]
      );

      mkQmkCli =
        name: source:
        pkgs.writeShellScript name ''
          export PYTHONPATH=${source}/lib/python
          exec ${qmkPython}/bin/python -c 'from qmk.cli import cli; cli()' "$@"
        '';

      mkFirmware =
        {
          name,
          tree,
          keyboard,
          ext,
          makeArgs ? [ ],
          rawHid64 ? false,
          perKeyValueScale ? false,
        }:
        let
          cli = mkQmkCli "qmk-${name}" tree;
        in
        pkgs.stdenv.mkDerivation {
          inherit name;
          src = tree;
          nativeBuildInputs = [
            qmkPython
            pkgs.gnumake
            pkgs.gcc-arm-embedded
            pkgs.dfu-util
          ];
          dontConfigure = true;
          dontFixup = true;
          buildPhase = ''
            export HOME=$TMPDIR SKIP_GIT=yes SKIP_VERSION=1
            export PYTHONPATH=$PWD/lib/python
            export ORIG_CWD=$PWD QMK_HOME=$PWD
            chmod -R u+w .
            ${pkgs.lib.optionalString rawHid64 "sed -i 's/#define RAW_EPSIZE 32/#define RAW_EPSIZE 64/' tmk_core/protocol/usb_descriptor.h && grep -q 'define RAW_EPSIZE 64' tmk_core/protocol/usb_descriptor.h"}
            ${pkgs.lib.optionalString perKeyValueScale "sed -i 's/hsv.v = rgb_matrix_config.hsv.v;/hsv.v = scale8(hsv.v, rgb_matrix_config.hsv.v);/' keyboards/keychron/common/rgb/per_key_rgb.c && grep -q 'scale8(hsv.v, rgb_matrix_config.hsv.v)' keyboards/keychron/common/rgb/per_key_rgb.c"}
            (cd ${./keyboards} && find . -type d -regex '.*/keymaps/[^/]*') | xargs -I{} rm -rf keyboards/{}
            cp -r --no-preserve=mode ${./keyboards}/. keyboards/
            sed -i 's/ --no-resolve-defaults//g' Makefile
            make ${builtins.concatStringsSep " " makeArgs} \
              QMK_BIN=${cli} \
              UF2CONV="${qmkPython}/bin/python util/uf2conv.py" \
              ${keyboard}:default
          '';
          installPhase = ''
            mkdir -p $out
            cp .build/*.${ext} $out/${name}.${ext}
          '';
        };
    in
    {
      packages.${system} = {
        k5-max = mkFirmware {
          name = "k5-max";
          tree = keychron-qmk;
          keyboard = "keychron/k5_max/ansi/rgb";
          ext = "bin";
          perKeyValueScale = true;
        };

        corne = mkFirmware {
          name = "corne";
          tree = qmk;
          keyboard = "splitkb/aurora/corne/rev1";
          ext = "uf2";
          makeArgs = [ "CONVERT_TO=liatris" ];
          rawHid64 = true;
        };
      };
    };
}
